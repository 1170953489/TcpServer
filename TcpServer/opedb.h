#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QReadWriteLock>
#include <QSqlQuery>
#include <QStringList>

class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);

    ~OpeDB();
    void init();

    int handleRegist(const char *name, const char *pwd);
    int handleCancell(const char *name, const char *pwd);
    int handleLogin(const char *name, const char *pwd);
    void handleOffline(const char *name);
    QStringList handleCheckOnlineUsr();
    int handleSerachUsr(const char *name);
    int handleCheckFriend(const char *pername,const char *name);
    void handleAddFriend(const char *pername,const char *name);
    QStringList handleFlushFriend(const char *name);
    bool handleDelFriend(const char *name,const char *friendName);

    void handleServerOffline();


signals:

public slots:

private:
    static int num;
    QSqlDatabase m_db;  //连接数据库
    QReadWriteLock dbLock;
};

#endif // OPEDB_H
