#ifndef DBMANAGER_H
#define DBMANAGER_H



class DBManager {
public:
    DBManager(const std::string& host, const std::string& user, const std::string& password, const std::string& db);
    ~DBManager();

    bool connect();
    bool createAccount(const std::string& username, const std::string& password);
    bool login(const std::string& username, const std::string& password);
    std::string getProfileJson(const std::string& username);

private:
    std::string m_strHost;
    std::string m_strUser;
    std::string m_strPW;
    std::string m_strDB;

    sql::Driver* m_Driver;
    std::unique_ptr<sql::Connection> m_Connection;
};

#endif // DBMANAGER_H


