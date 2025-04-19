#ifndef DBWORKER_H
#define DBWORKER_H

#include "stdafx.h"
#include "DBTask.h"

class DBWorker
{
public:
    DBWorker() : DBWorker(4) {}
    DBWorker(int numThreads);
    ~DBWorker();

    void start();
    void stop();

    void enqueueTask(const DBTask& task);

private:
    void workerThread();

    std::queue<DBTask> m_Tasks;
    std::mutex m_TasksMutex;
    std::condition_variable m_TasksCV;
    std::atomic<bool> m_StopFlag;
    std::vector<std::thread> m_Threads;
};

extern DBWorker g_dbWorker;

#endif // DBWORKER_H

