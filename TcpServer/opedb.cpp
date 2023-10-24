#include "opedb.h"
#include <QMessageBox>
#include <QDebug>

int OpeDB::num = 0;

OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    QString strName = QString("connection%1").arg(QString::number(++num));
    m_db = QSqlDatabase::addDatabase("QSQLITE", strName);
    m_db.setDatabaseName("cloud.db");
    if (!m_db.open()) QMessageBox::critical(nullptr, "打开数据库", "打开数据库失败");
}

OpeDB::~OpeDB()
{
    m_db.close();
}

void OpeDB::init()
{
    QSqlQuery query(m_db);
    query.exec("SELECT name FROM sqlite_master WHERE type='table'");
    if (!query.next())
    {
        query.exec("create table usrInfo(id integer primary key autoincrement, name varchar(32) unique, pwd varchar(32), online integer default 0)");
        query.exec("create table 'friend'(id integer, friendId integer, primary key(id, friendId))");
    }
}

int OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (name == nullptr || pwd == nullptr) return false;
    dbLock.lockForWrite();

    QString data = QString("insert into usrInfo(name, pwd) values('%1','%2')").arg(name).arg(pwd);
    QSqlQuery query(m_db);
    bool ret = query.exec(data);

    dbLock.unlock();
    return ret;
}

int OpeDB::handleCancell(const char *name, const char *pwd)
{
    if (name == nullptr || pwd == nullptr) return false;
    dbLock.lockForWrite();

    QString data = QString("select * from usrInfo where name='%1' and pwd='%2' and online=0").arg(name).arg(pwd);
    QSqlQuery query(m_db);
    query.exec(data);
    if(!query.next())
    {
        data = QString("select * from usrInfo where name='%1' and pwd='%2' and online=1").arg(name).arg(pwd);
        query.exec(data);
        if (query.next())
        {
            dbLock.unlock();
            return 1;
        }
        else
        {
            dbLock.unlock();
            return -1;
        }
    }
    int id = query.value(0).toInt();
    data = QString("delete from usrInfo where name='%1' and pwd='%2'").arg(name).arg(pwd);
    query.exec(data);

    data = QString("delete from friend where id='%1' or firendId='%2'").arg(QString::number(id)).arg(QString::number(id));
    query.exec(data);

    dbLock.unlock();
    return 0;
}

int OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (name == nullptr || pwd == nullptr) return false;
    dbLock.lockForWrite();

    QString data = QString("select * from usrInfo where name='%1' and pwd='%2' and online=0").arg(name).arg(pwd);
    QSqlQuery query(m_db);
    query.exec(data);
    if (query.next())
    {
        data = QString("update usrInfo set online=1 where name='%1' and pwd='%2'").arg(name).arg(pwd);
        QSqlQuery query(m_db);
        query.exec(data);

        dbLock.unlock();
        return 0;
    }
    else
    {
        query.clear();
        data = QString("select * from usrInfo where name='%1' and pwd='%2' and online=1").arg(name).arg(pwd);
        query.exec(data);
        if (query.next())
        {
            dbLock.unlock();
            return 1;
        }
        else
        {
            dbLock.unlock();
            return -1;
        }
    }
}

void OpeDB::handleOffline(const char *name)
{
    if (name == nullptr) return;
    dbLock.lockForWrite();

    QString data = QString("update usrInfo set online=0 where name='%1'").arg(name);
    QSqlQuery query(m_db);
    query.exec(data);

    dbLock.unlock();
    return;
}

QStringList OpeDB::handleCheckOnlineUsr()
{
    dbLock.lockForRead();

    QString data = QString("select name from usrInfo where online=1");
    QSqlQuery query(m_db);
    query.exec(data);

    dbLock.unlock();
    QStringList result;
    result.clear();

    while (query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSerachUsr(const char *name)
{
    if (name == nullptr) return -1;
    dbLock.lockForRead();

    QString data = QString("select online from usrInfo where name='%1'").arg(name);
    QSqlQuery query(m_db);
    query.exec(data);

    dbLock.unlock();
    if (query.next())
    {
        int ret = query.value(0).toInt();
        return ret;
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleCheckFriend(const char *pername, const char *name)
{
    if (pername == nullptr || name == nullptr) return -1;
    dbLock.lockForRead();

    QString data = QString("select * from friend where (id=(select id from usrInfo where name='%1') and friendId=(select id from usrInfo where name='%2')) "
                           "or (id=(select id from usrInfo where name='%3') and friendId=(select id from usrInfo where name='%4'))").arg(pername).arg(name).arg(name).arg(pername);
    QSqlQuery query(m_db);
    query.exec(data);
    if (query.next())
    {
        dbLock.unlock();
        return 0;   // 双方已经是好友
    }
    else
    {
        data = QString("select online from usrInfo where name='%1'").arg(pername);
        QSqlQuery query(m_db);
        query.exec(data);

        dbLock.unlock();
        if (query.next())
        {
            int ret = query.value(0).toInt();
            if (ret == 1)
            {
                return 1;   // 对方在线可添加好友
            }
            else
            {
                return 2;   //  对方不在线不可添加好友
            }
        }
        else
        {
            return 3;   // 用户不存在
        }
    }
}

void OpeDB::handleAddFriend(const char *pername, const char *name)
{
    dbLock.lockForWrite();

    QString data = QString("insert into friend(id, friendId) values((select id from usrInfo where name='%1'),(select id from usrInfo where name='%2'))").arg(name).arg(pername);
    QSqlQuery query(m_db);
    query.exec(data);

    dbLock.unlock();
    return;
}

QStringList OpeDB::handleFlushFriend(const char *name)
{
    QStringList strFriendList;
    strFriendList.clear();
    if(name == nullptr) return strFriendList;
    dbLock.lockForRead();

    QString data = QString("select name from usrInfo where id in (select id from friend where friendId=(select id from usrInfo where name='%1'))").arg(name);
    QSqlQuery query(m_db);
    query.exec(data);
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
    }
    query.clear();
    data = QString("select name from usrInfo where id in (select friendId from friend where id=(select id from usrInfo where name='%1'))").arg(name);
    query.exec(data);

    dbLock.unlock();
    while (query.next())
    {
        strFriendList.append(query.value(0).toString());
    }
    return strFriendList;
}

bool OpeDB::handleDelFriend(const char *name, const char *friendName)
{
    if (name == nullptr || friendName == nullptr) return false;
    dbLock.lockForWrite();

    QString data = QString("delete from friend where id=(select id from usrInfo where name='%1') and friendId=(select id from usrInfo where name='%2')"
                           " or id=(select id from usrInfo where name='%3') and friendId=(select id from usrInfo where name='%4')").arg(name).arg(friendName).arg(friendName).arg(name);
    QSqlQuery query(m_db);
    query.exec(data);

    dbLock.unlock();
    return true;
}

void OpeDB::handleServerOffline()
{
    dbLock.lockForWrite();

    QString data = QString("update usrInfo set online=0");
    QSqlQuery query(m_db);
    query.exec(data);

    dbLock.unlock();
    return;
}
