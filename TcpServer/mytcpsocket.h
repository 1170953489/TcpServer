#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"
#include <QDir>
#include <QFile>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    ~MyTcpSocket();
    QString getStrName();

signals:
    void offline(MyTcpSocket *mysocket);
    void closeThread();

public slots:
    void resendMsg(QByteArray);
    void recvMsg();
    void clientOffline();

private:
    QByteArray buffer;  //缓存收到的数据
    QString m_strName;  //当前登录用户名
    OpeDB *m_db;        //连接数据库
};

#endif // MYTCPSOCKET_H
