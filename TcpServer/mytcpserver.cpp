#include "mytcpserver.h"
#include <QDebug>
#include <QThread>

MyTcpServer::MyTcpServer()
{

}

MyTcpServer &MyTcpServer::getInstance()
{
    static  MyTcpServer instance;
    return instance;
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    QThread* thread = new QThread;
    MyTcpSocket *pTcpSocket = new MyTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    pTcpSocket->moveToThread(thread);

    m_tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, &MyTcpSocket::offline, this, &MyTcpServer::deletSocket);
    connect(pTcpSocket, &MyTcpSocket::closeThread, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, this, &MyTcpServer::on_ThreadFinished);

    thread->start();

}

void MyTcpServer::resend(const char *pername, PDU *pdu)
{
    if (pername == nullptr || pdu == nullptr) return;
    QString strPerName = pername;
    for (int i = 0; i < m_tcpSocketList.size(); ++i)
    {
        if (strPerName == m_tcpSocketList.at(i)->getStrName())
        {
            QByteArray byteArray((char*)pdu, pdu->uiPDULen);
            QMetaObject::invokeMethod(m_tcpSocketList.at(i), "resendMsg",
                                      Qt::QueuedConnection,
                                      QGenericArgument("QByteArray", &byteArray));
            break;
        }
    }
}

void MyTcpServer::deletSocket(MyTcpSocket *mysocket)
{

    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for (; iter != m_tcpSocketList.end(); ++iter)
    {
        if (mysocket == *iter)
        {
            (*iter)->deleteLater();
            *iter = nullptr;
            m_tcpSocketList.erase(iter);
            qDebug() << "MyTcpSocket已移除list";
            return;
        }
    }
}

void MyTcpServer::on_ThreadFinished()
{
    qDebug() << "MyTcpServer Thread has finished!";
}
