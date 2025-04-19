#pragma once

#include "stdafx.h"

class CClient;

enum class OperationType
{
    RECV,
    SEND
};

// Overlapped structure for IOCP
struct OverlappedEx
{
    OVERLAPPED overlapped;
    CClient* client;
    OperationType operation;
};

class CClient
{
public:
    CClient(SOCKET socket);
    ~CClient();

    void Start();        
    void ReceiveCompleted(DWORD bytesTransferred);
    void SendCompleted(DWORD bytesTransferred);
    void SendAuthResponse(Net::AuthType type, Net::ResultCode result, const std::string& msg);
    void HandleProfileRequest(const Net::ProfileRequest* req);

private:
    void Send(const flatbuffers::FlatBufferBuilder& builder);
    void SendProfileResponse(const std::string& profileJson);
    void HandleLogin(const std::string& username, const std::string & password);
    std::string generateToken();

public:
    SOCKET m_socket;
    OverlappedEx m_recvOverlapped;
    OverlappedEx m_sendOverlapped;
    WSABUF m_recvBuffer;
    WSABUF m_sendBuffer;
};