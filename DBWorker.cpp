#include "DBWorker.h"
#include "DBManager.h"
#include <iostream>

DBWorker g_dbWorker(4);

DBWorker::DBWorker(int numThreads)
    : m_StopFlag(false)
{
    m_Threads.reserve(numThreads);
}

DBWorker::~DBWorker()
{
    stop();
}

void DBWorker::start()
{
    for (size_t i = 0; i < m_Threads.capacity(); ++i)
        m_Threads.emplace_back(&DBWorker::workerThread, this);
}

void DBWorker::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_TasksMutex);
        m_StopFlag = true;
    }
    m_TasksCV.notify_all();

    for (std::thread& t : m_Threads)
    {
        if (t.joinable())
            t.join();
    }
    m_Threads.clear();
}

void DBWorker::enqueueTask(const DBTask& task)
{
    {
        std::unique_lock<std::mutex> lock(m_TasksMutex);
        m_Tasks.push(task);
    }
    m_TasksCV.notify_one();
}

void DBWorker::workerThread()
{
    DBManager dbManager("tcp://127.0.0.1:3600", "db_test", "db_test", "my_test");
    if (!dbManager.connect())
    {
        std::cerr << "[DBWorker] DB connect fail" << std::endl;
        return;
    }

    while (true)
    {
        DBTask _task;
        {
            std::unique_lock<std::mutex> lock(m_TasksMutex);
            m_TasksCV.wait(lock, [this]() { return m_StopFlag || !m_Tasks.empty(); });
            if (m_StopFlag && m_Tasks.empty())
                return;

            _task = m_Tasks.front();
            m_Tasks.pop();
        }

        bool result = false;
        if (_task.m_Type == DBRequestType::CREATE_ACCOUNT)
            result = dbManager.createAccount(_task.m_strUsername, _task.m_strPW);
        else if (_task.m_Type == DBRequestType::LOGIN)
            result = dbManager.login(_task.m_strUsername, _task.m_strPW);

        if (_task.m_Callback)
            _task.m_Callback(result);
    }
}
