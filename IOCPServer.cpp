#include "stdafx.h"

std::vector<CClient*> g_clients;
std::queue<CClient*> g_freeClients;
std::mutex g_freeClientsMutex;
std::condition_variable g_freeClientsCV;
HANDLE g_completionPort;

void WorkerThread()
{
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    OverlappedEx* overlapped;
    while (true)
    {
        BOOL result = GetQueuedCompletionStatus(g_completionPort, &bytesTransferred, &completionKey, (LPOVERLAPPED*)&overlapped, INFINITE);
        if (!result || bytesTransferred == 0)
        {
            // Client disconnected or error occurred
            if (overlapped)
            {
                CClient* client = overlapped->client;
                delete overlapped;
                delete client;
            }
            continue;
        }

        // IOCP operation completed successfully
        switch (overlapped->operation)
        {
        case OperationType::RECV:
            overlapped->client->ReceiveCompleted(bytesTransferred);
            break;
        case OperationType::SEND:
            overlapped->client->SendCompleted(bytesTransferred);
            break;
        }
    }
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    g_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_completionPort == NULL)
    {
        std::cerr << "CreateIoCompletionPort failed with error " << GetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i)
        threads.emplace_back(WorkerThread);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed with error " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "bind failed with error " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen failed with error " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    g_dbWorker.start();
    std::cout << "Server Running......" << std::endl;

    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "accept failed with error " << WSAGetLastError() << std::endl;
            continue;
        }

        // Create client object
        CClient* client = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_freeClientsMutex);
            if (!g_freeClients.empty())
            {
                client = g_freeClients.front();
                g_freeClients.pop();
            }
        }

        if (!client)
            client = new CClient(clientSocket);
        else
            client->m_socket = clientSocket;

        if (!CreateIoCompletionPort((HANDLE)client->m_socket, g_completionPort, (ULONG_PTR)client, 0))
        {
            std::cerr << "CreateIoCompletionPort failed with error " << GetLastError() << std::endl;
            delete client;
            continue;
        }
 
        client->Start();
        
    }

    closesocket(listenSocket);
    WSACleanup();

    for (auto& thread : threads)
    {
        thread.join();
    }

    g_dbWorker.stop();
    return 0;
}
