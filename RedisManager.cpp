#include "RedisManager.h"

using namespace sw::redis;

RedisManager g_redis("tcp://127.0.0.1:6379");

RedisManager::RedisManager(const std::string& uri)
    : m_Redis(Redis(uri))
{
}

RedisManager::~RedisManager()
{
}

bool RedisManager::createSession(const std::string& sessionId, const std::string& userInfo, int ttlSeconds)
{
    try
    {
        m_Redis.set(sessionId, userInfo);
        m_Redis.expire(sessionId, ttlSeconds);
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] createSession error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

std::optional<std::string> RedisManager::getSession(const std::string& sessionId)
{
    try
    {
        auto val = m_Redis.get(sessionId);
        if (val)
            return *val;
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] getSession error: " << e.what() << std::endl;
    }
    return std::nullopt;
}

bool RedisManager::deleteSession(const std::string& sessionId)
{
    try
    {
        m_Redis.del(sessionId);
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] deleteSession error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool RedisManager::cacheUserData(const std::string& userId, const std::string& data, int ttlSeconds)
{
    try
    {
        m_Redis.set(userId, data);
        m_Redis.expire(userId, ttlSeconds);
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] cacheUserData error: " << e.what() << std::endl;
        return false;
    }
    return true;
}

std::optional<std::string> RedisManager::getUserData(const std::string& userId)
{
    try
    {
        auto val = m_Redis.get(userId);
        if (val)
        {
            return *val;
        }
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] getUserData error: " << e.what() << std::endl;
    }
    return std::nullopt;
}

// username_failed 키를 사용하여 로그인 실패 횟수를 관리
int RedisManager::incrementFailedLogin(const std::string& username, int ttlSeconds)
{
    try
    {
        std::string key = username + ":failed";
        auto count = m_Redis.incr(key);
        // 첫 번째 실패 시 TTL 설정
        if (count == 1) {
            m_Redis.expire(key, ttlSeconds);
        }
        return static_cast<int>(count);
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] incrementFailedLogin error: " << e.what() << std::endl;
        return -1;
    }
}

void RedisManager::clearFailedLogin(const std::string& username)
{
    try
    {
        std::string key = username + ":failed";
        m_Redis.del(key);
    }
    catch (const Error& e)
    {
        std::cerr << "[RedisManager] clearFailedLogin error: " << e.what() << std::endl;
    }
}
