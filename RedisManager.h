#pragma once

#include "stdafx.h"

class RedisManager
{
public:
    RedisManager(const std::string& uri);
    ~RedisManager();

    //���� ����
    bool createSession(const std::string& sessionId, const std::string& userInfo, int ttlSeconds);
    std::optional<std::string> getSession(const std::string& sessionId);
    bool deleteSession(const std::string& sessionId);

    //����� ������ ĳ��
    bool cacheUserData(const std::string& userId, const std::string& data, int ttlSeconds);
    std::optional<std::string> getUserData(const std::string& userId);

    //�α��� �õ� ����
    int incrementFailedLogin(const std::string& username, int ttlSeconds);
    void clearFailedLogin(const std::string& username);

private:
    sw::redis::Redis m_Redis;
};

extern RedisManager g_redis;
