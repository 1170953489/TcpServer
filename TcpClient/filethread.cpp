#include "filethread.h"
#include "tcpclient.h"
#include <QHostAddress>
#include <QThread>

FileThread::FileThread(QString str, QByteArray byteArray, QObject *parent) : QObject(parent)
{
    m_strLocalFilePath = str;
    m_byteArray = byteArray;
    pFileTransfer = nullptr;
    qDebug() << "FileThread 生成";
}

FileThread::~FileThread()
{
    qDebug() << "FileThread 销毁";
}

FileTransfer *FileThread::getfileTransfer()
{
    return pFileTransfer;
}
void FileThread::init()
{
    pFileTransfer = new FileTransfer(m_strLocalFilePath, this);
    connect(pFileTransfer, &FileTransfer::connected, this, &FileThread::onConnected);
    pFileTransfer->connectToHost(QHostAddress(TcpClient::getInstance()->curIp()), 9999);

    connect(this, &FileThread::sendMsg, pFileTransfer, &FileTransfer::sendMsg);
    connect(pFileTransfer, &FileTransfer::upDownloadProgress, this, &FileThread::resendProgress);
    connect(pFileTransfer, &FileTransfer::workFinished, this, &FileThread::finish);
}

void FileThread::onConnected()
{
    if (pFileTransfer->state() == QAbstractSocket::ConnectedState)
        sendMsg(m_byteArray);

}

void FileThread::resendProgress(qint64 m_iRecived, qint64 m_iTotal)
{
    emit upProgress(m_iRecived, m_iTotal);
}

void FileThread::finish()
{
    if(nullptr != pFileTransfer)
    {
        pFileTransfer->close();
        pFileTransfer->deleteLater();
    }
    emit over();
}
