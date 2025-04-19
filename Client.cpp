
#include "Client.h"
#include "DBManager.h"
#include "DBTask.h"
#include "RedisManager.h"


CClient::CClient(SOCKET socket) : m_socket(socket)
{
	m_recvBuffer.len = BUFFER_SIZE;
    m_recvBuffer.buf = new char[BUFFER_SIZE];
	m_sendBuffer.len = BUFFER_SIZE;
    m_sendBuffer.buf = new char[BUFFER_SIZE];
}

CClient::~CClient()
{
	closesocket(m_socket);
}

void CClient::Start()
{
    // Post initial receive operation
    memset(&m_recvOverlapped, 0, sizeof(m_recvOverlapped));
    DWORD flags = 0; 
    DWORD bytesRecv = 0;
    m_recvOverlapped.operation = OperationType::RECV;
    m_recvOverlapped.client = this;

    if (WSARecv(m_socket, &m_recvBuffer, 1, &bytesRecv, &flags, &m_recvOverlapped.overlapped, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
              std::cerr << "WSARecv failed with error " << WSAGetLastError() << std::endl;
            delete this;
        }
    }
}

void CClient::SendAuthResponse(Net::AuthType type, Net::ResultCode result, const std::string& msg)
{
    flatbuffers::FlatBufferBuilder builder;

    auto fb_msg = builder.CreateString(msg);
    auto response = Net::CreateAuthResponse(builder, type, result, fb_msg);
    builder.Finish(response);

    Send(builder);  // 기존 Send 함수 호출
}

void CClient::Send(const flatbuffers::FlatBufferBuilder& builder)
{
    // Serialize Flatbuffer
    const uint8_t* buf = builder.GetBufferPointer();
    int size = builder.GetSize();

    // Copy serialized data to send buffer
    std::copy(buf, buf + size, m_sendBuffer.buf);

    // Post send operation
    memset(&m_sendOverlapped, 0, sizeof(m_sendOverlapped));
    DWORD bytesSent = 0;
    m_sendOverlapped.operation = OperationType::SEND;
    if (WSASend(m_socket, &m_sendBuffer, 1, &bytesSent, 0, &m_sendOverlapped.overlapped, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            std::cerr << "WSASend failed with error " << WSAGetLastError() << std::endl;
            delete this;
        }
    }
}

void CClient::ReceiveCompleted(DWORD bytesTransferred)
{
    const uint8_t* buffer = reinterpret_cast<const uint8_t*>(m_recvBuffer.buf);
    auto packet = Net::GetAuthPacket(buffer);

    switch (packet->type())
    {
    case Net::AuthType_CREATE:
    {
        auto req = packet->create();
        std::string account = req->username()->str();
        std::string pw = req->password()->str();

        std::cout <<"Create Account" << "acount:" << account.c_str() << "\t" << "pw:" << pw.c_str() << std::endl;

        HandleLogin(account, pw);
    }
    break;
    case Net::AuthType_LOGIN:
    {
        auto req = packet->login();
        std::string account = req->username()->str();
        std::string pw = req->password()->str();

        std::cout << "Login user:"<< account.c_str() << std::endl;

        DBTask loginTask;
        loginTask.m_Type = DBRequestType::LOGIN;
        loginTask.m_strUsername = "exampleUser";
        loginTask.m_strPW = "examplePass";
        loginTask.m_Callback = [](bool bRet)
            {
                if (bRet)
                    std::cout << "[DBWorker] 로그인 성공" << std::endl;
                else
                    std::cout << "[DBWorker] 로그인 실패" << std::endl;
            };
        g_dbWorker.enqueueTask(loginTask);

        SendAuthResponse(Net::AuthType_LOGIN, Net::ResultCode_SUCCESS, "Login success");
    }
    break;
    case Net::AuthType_LOGOUT:
    {
        auto req = packet->logout();
        std::string account = req->username()->str();

        std::cout << "Logout user:" << account.c_str() << std::endl;

        SendAuthResponse(Net::AuthType_LOGOUT, Net::ResultCode_SUCCESS, "Logout success");
    }
    break;
    default:
        break;
    }


    memset(&m_recvOverlapped, 0, sizeof(m_recvOverlapped));
    DWORD flags = 0;
    DWORD bytesRecv = 0;
    m_recvOverlapped.operation = OperationType::RECV;
    m_recvOverlapped.client = this;
    if (WSARecv(m_socket, &m_recvBuffer, 1, &bytesRecv, &flags, &m_recvOverlapped.overlapped, NULL) == SOCKET_ERROR)
    {
        if (WSAGetLastError() != WSA_IO_PENDING)
        {
            std::cerr << "WSARecv failed with error " << WSAGetLastError() << std::endl;
            delete this;
        }
    }
}

void CClient::SendCompleted(DWORD bytesTransferred)
{
    // Send operation completed
}

std::string CClient::generateToken()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i)
        ss << std::setw(8) << dist(gen);

    return ss.str();
}

void CClient::HandleLogin(const std::string& username, const std::string& password)
{
    int fails = g_redis.incrementFailedLogin(username, 600);
    if (fails > 5)
    {
        SendAuthResponse(Net::AuthType::AuthType_LOGIN, Net::ResultCode::ResultCode_TOO_MANY_ATTEMPTS, "Too many failed attempts");
        return;
    }

    DBTask task;
    task.m_Type = DBRequestType::LOGIN;
    task.m_strUsername = username;
    task.m_strPW = password;
    task.m_Callback = [this, username](bool success){
        if (!success)
        {
            SendAuthResponse(Net::AuthType::AuthType_LOGIN, Net::ResultCode::ResultCode_FAIL, "");
            return;
        }

        // 로그인 성공 시 실패 카운터 삭제
        g_redis.clearFailedLogin(username);

        // 세션 생성
        std::string token = generateToken();
        g_redis.createSession(token, username, 3600);      // 1시간 TTL

        SendAuthResponse(Net::AuthType::AuthType_LOGIN, Net::ResultCode::ResultCode_SUCCESS, token);
        };

    g_dbWorker.enqueueTask(task);
}

void CClient::HandleProfileRequest(const Net::ProfileRequest* req)
{
    std::string username = req->username()->str();

    auto cached = g_redis.getUserData(username);
    if (cached)
    {
        SendProfileResponse(*cached);
        return;
    }

    DBManager dbMgr("tcp://127.0.0.1:3301", "db_test", "db_test", "my_test");
    if (!dbMgr.connect())
    {
        SendProfileResponse("{}");
        return;
    }
    std::string profileJson = dbMgr.getProfileJson(username);

    g_redis.cacheUserData(username, profileJson, 300);

    SendProfileResponse(profileJson);
}

void CClient::SendProfileResponse(const std::string& profileJson)
{
    flatbuffers::FlatBufferBuilder fb;
    auto jsonOffset = fb.CreateString(profileJson);
    auto resp = Net::CreateProfileResponse(fb, jsonOffset);
    fb.Finish(resp);
    Send(fb);
}