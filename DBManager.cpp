#include "stdafx.h"
#include "DBManager.h"

DBManager::DBManager(const std::string& host, const std::string& user, const std::string& password, const std::string& db)
    : m_strHost(host), m_strUser(user), m_strPW(password), m_strDB(db), m_Driver(nullptr)
{
}

DBManager::~DBManager()
{
}

bool DBManager::connect()
{
    try
    {
        m_Driver = get_driver_instance();
        m_Connection.reset(m_Driver->connect(m_strHost, m_strUser, m_strPW));
        m_Connection->setSchema(m_strDB);
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "MySQL 연결 실패: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DBManager::createAccount(const std::string& username, const std::string& password)
{
    try
    {
        // 예제: 비밀번호는 실제 구현 시 적절한 해싱 과정을 거쳐야 함
        std::unique_ptr<sql::PreparedStatement> pstmt(
            m_Connection->prepareStatement("INSERT INTO accounts (username, password, ip) VALUES (?, ?)"));
        pstmt->setString(1, username);
        pstmt->setString(2, password);
        pstmt->executeUpdate();
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "계정 생성 실패: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool DBManager::login(const std::string& username, const std::string& password)
{
    try
    {
        std::unique_ptr<sql::PreparedStatement> pstmt(
            m_Connection->prepareStatement("SELECT password FROM accounts WHERE username = ?"));
        pstmt->setString(1, username);
        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
        if (!res->next()) {
            // 계정이 존재하지 않음
            return false;
        }
        std::string storedHash = res->getString("password");

        // 입력받은 password와 storedHash를 비교합니다.
        // 실제 구현 시, 해시 비교 함수를 호출해야 함 (예: bcrypt, Argon2 등)
        if (storedHash == password)
        {  // 단순 비교; 실제 구현에서는 해시 비교 필요
            return true;
        }
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "로그인 실패: " << e.what() << std::endl;
        return false;
    }
    return false;
}

std::string DBManager::getProfileJson(const std::string& username)
{
    try
    {
        auto pstmt = std::unique_ptr<sql::PreparedStatement>(
            m_Connection->prepareStatement("SELECT email, created_at FROM accounts WHERE username = ?")); 
        pstmt->setString(1, username);
        auto res = std::unique_ptr<sql::ResultSet>(pstmt->executeQuery());
        if (res->next())
        {
            std::ostringstream oss;
            oss << "{"
                << "\"username\":\"" << username << "\","
                << "\"email\":\"" << res->getString("email") << "\","
                << "\"created_at\":\"" << res->getString("created_at") << "\""
                << "}";
            return oss.str();
        }
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[MySQLManager] getProfileJson error: " << e.what() << std::endl;
    }
    return "{}";
}