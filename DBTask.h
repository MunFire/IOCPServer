#ifndef DBTASK_H
#define DBTASK_H

#include <string>
#include <functional>

enum class DBRequestType
{
    CREATE_ACCOUNT,
    LOGIN
};

struct DBTask {
    DBRequestType m_Type;
    std::string m_strUsername;
    std::string m_strPW;
    
    std::function<void(bool)> m_Callback;     // �۾� �Ϸ� �� ����� ó���ϴ� �ݹ�
};

#endif // DBTASK_H

