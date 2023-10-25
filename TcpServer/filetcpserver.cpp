#include "filetcpserver.h"
#include "mytcpserver.h"
#include <QThread>
#include <QTimer>

FileTcpServer::FileTcpServer()
{

}

FileTcpServer &FileTcpServer::getInstance()
{
    static FileTcpServer instance;
    return instance;
}

void FileTcpServer::incomingConnection(qintptr socketDescriptor)
{
    QThread* thread = new QThread;
    FileTcpSocket *pFileSocket = new FileTcpSocket;
    pFileSocket->moveToThread(thread);
    m_fileSocketList.append(pFileSocket);

    connect(pFileSocket, &FileTcpSocket::sendProgress, [=](qint64 m_iRecived, qint64 m_iTotal) {
        if (double(m_iRecived)/m_iTotal*100 - pFileSocket->m_curProgress >= 0.001 || m_iTotal == m_iRecived)
        {
            pFileSocket->m_curProgress = double(m_iRecived)/m_iTotal*100;
            PDU *respud = mkPDU(sizeof(int));
            respud->uiMsgType = ENUM_MSG_TYPE_SEND_FILE_PROGRESS;
            memcpy(respud->caData, &m_iTotal, sizeof(qint64));
            memcpy(respud->caData+32, &m_iRecived, sizeof(qint64));
            int number = pFileSocket->getNumber();
            memcpy(respud->caMsg, &number, sizeof(int));
            MyTcpServer::getInstance().resend(pFileSocket->getUsrName().toStdString().c_str(), respud);

            free(respud);
            respud = nullptr;
        }
    });
    connect(pFileSocket, &FileTcpSocket::deleteSocket, this, &FileTcpServer::deleteSocket);

    connect(pFileSocket, &FileTcpSocket::closeThread, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, this, &FileTcpServer::on_ThreadFinished);

    thread->start();

    QTimer::singleShot(1, pFileSocket, [pFileSocket, socketDescriptor]() {
        pFileSocket->init(socketDescriptor);
    });
}

void FileTcpServer::uploadCancel(int number)
{
    for (int i = 0; i < m_fileSocketList.size(); ++i)
    {
        if (number == m_fileSocketList.at(i)->getNumber())
        {
            qDebug() << "找到文件编号 " << number;
            m_fileSocketList.at(i)->uploadCancel();
            break;
        }
    }
}

void FileTcpServer::downloadCancel(int number)
{
    for (int i = 0; i < m_fileSocketList.size(); ++i)
    {
        if (number == m_fileSocketList.at(i)->getNumber())
        {
            m_fileSocketList.at(i)->isStop = true;
            usleep(1000);
            m_fileSocketList.at(i)->workFinished();
            break;
        }
    }
}


void FileTcpServer::on_ThreadFinished()
{
    qDebug() << "FileTcpServer Thread has finished!";
}

void FileTcpServer::deleteSocket(FileTcpSocket *socket)
{
    QList<FileTcpSocket*>::iterator iter = m_fileSocketList.begin();
    for (; iter != m_fileSocketList.end(); ++iter)
    {
        if (socket == *iter)
        {
            (*iter)->deleteLater();
            *iter = nullptr;
            m_fileSocketList.erase(iter);
            return;
        }
    }
}
