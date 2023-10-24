#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"



class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static MyTcpServer& getInstance();
    virtual void incomingConnection(qintptr socketDescriptor) override;
    void resend(const char *pername, PDU *pdu);

public slots:
    void deletSocket(MyTcpSocket *mysocket);
    void on_ThreadFinished();

private:
    MyTcpServer();
    QList<MyTcpSocket*> m_tcpSocketList;
};

#endif // MYTCPSERVER_H
