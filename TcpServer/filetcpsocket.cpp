#include "filetcpsocket.h"
#include <QFileInfoList>

FileTcpSocket::FileTcpSocket()
{
    qDebug() << "\nFileTcpSocket生成";
    isStop = false;
}

FileTcpSocket::~FileTcpSocket()
{    
    if (nullptr != socket)
    {
        socket->close();
        socket->deleteLater();
    }
    emit closeThread();
    qDebug() << "FileTcpSocket销毁";
}

int FileTcpSocket::getNumber()
{
    return number;
}

QString FileTcpSocket::getUsrName()
{
    return m_usrName;
}

void FileTcpSocket::uploadCancel()
{
    m_file.close();
    m_file.remove();

    emit workFinished();
}

void FileTcpSocket::init(qintptr socketDescriptor)
{
    socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    connect(this, &FileTcpSocket::workFinished, this, &FileTcpSocket::clientOffline);
}

void FileTcpSocket::recvMsg()
{
    buffer.append(socket->readAll());
    while (buffer.size() > 0) {

        PDU *pdu = nullptr;
        uint uiPDULen = 0;
        memcpy((char*)&uiPDULen, buffer.constData(), sizeof(uint));
        if (uiPDULen > buffer.size()) break;
        else
        {
            uint uiMsgLen = uiPDULen - sizeof (PDU);
            pdu = mkPDU(uiMsgLen);
            memcpy(pdu, buffer.constData(), uiPDULen);
            buffer.remove(0, uiPDULen);
        }

        switch (pdu->uiMsgType)
        {
            // 收到上传文件请求
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            qDebug() << "------------------------收到上传文件请求------------------------";
            qint64 fileSize = 0;
            sscanf(pdu->caData, "%lld", &fileSize);
            memcpy(&number, pdu->caData+32, sizeof(int));
            QByteArray byteArray(pdu->caMsg, pdu->uiMsgLen);
            QString strNewPath = byteArray.data();

            qDebug() << "文件编号:" << number << "文件大小:" << fileSize << "Byte\n文件地址:" << strNewPath;
            qDebug() << "---------------------------------------------------------------";

            m_file.setFileName(strNewPath);
            // 以只写的方式打开文件，若文件不存在，则会自动创建文件
            if (m_file.open(QIODevice::WriteOnly))
            {
                m_iTotal = fileSize;
                m_usrName = strNewPath.mid(6);
                m_usrName = m_usrName.left(m_usrName.indexOf('/'));
            }
            else
            {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
                strcpy(respdu->caData, UPLOAD_FILE_FAILED);
                socket->write((char*)(respdu), respdu->uiPDULen);
                socket->flush();
                free(respdu);
                respdu = nullptr;
                free(pdu);
                pdu = nullptr;

                socket->waitForBytesWritten();
                emit workFinished();
                return;
            }
            break;
        }
            // 写入上传的文件
        case ENUM_MSG_TYPE_SEND_FILE:
        {
            if (m_file.isOpen()) // 如果文件是打开状态
            {
                QByteArray buff(pdu->caMsg, pdu->uiMsgLen);
                // 写入
                m_file.write(buff);
                m_file.flush();
                m_iRecived += buff.size();

                // 回传给客户端服务器的写入进度
                emit sendProgress(m_iRecived, m_iTotal);

                if (m_iTotal == m_iRecived)
                {
                    m_file.close();

                    PDU *respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
                    strcpy(respdu->caData, UPLOAD_FILE_OK);
                    socket->write((char*)(respdu), respdu->uiPDULen);
                    socket->flush();
                    free(respdu);
                    respdu = nullptr;

                    socket->waitForBytesWritten();
                    emit workFinished();
                    break;
                }
                else if (m_iTotal < m_iRecived) //失败
                {
                    m_file.remove();
                    m_file.close();

                    PDU *respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
                    strcpy(respdu->caData, UPLOAD_FILE_FAILED);
                    socket->write((char*)(respdu), respdu->uiPDULen);
                    socket->flush();
                    free(respdu);
                    respdu = nullptr;

                    socket->waitForBytesWritten();
                    emit workFinished();
                    break;
                }
            }
            break;
        }
            // 收到下载文件请求, 写出文件
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            qDebug() << "------------------------收到下载文件请求------------------------";
            memcpy(&number, pdu->caData, sizeof(int));
            QByteArray byteArray(pdu->caMsg, pdu->uiMsgLen);
            QString strDownloadPath = byteArray.data();
            m_file.setFileName(strDownloadPath);
            if (!m_file.open(QIODevice::ReadOnly))
            {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
                strcpy(respdu->caData, DOWNLOAD_FILE_FAILED);
                socket->write((char*)respdu, respdu->uiPDULen);
                socket->flush();
                free(respdu);
                respdu = nullptr;

                socket->waitForBytesWritten();
                emit workFinished();
                return;
            }
            m_iTotal = m_file.size();

            qDebug() << "文件编号:" << number << "文件大小:" << m_iTotal << "Byte\n文件地址:" << strDownloadPath;
            qDebug() << "---------------------------------------------------------------";

            PDU *m_pdu = mkPDU(1024000);
            m_pdu->uiMsgType = ENUM_MSG_TYPE_SEND_FILE;
            memcpy(m_pdu->caData, &m_iTotal, sizeof(qint64));
            uint ret = 0;
            // 开始将数据发出
            while (true)
            {
                if (isStop) //中途停止
                {
                    qDebug() << "中途停止下载";
                    break;
                }
                ret = m_file.read((char*)(m_pdu->caMsg), 1024000);
                if (ret > 0 && ret <= 1024000)
                {
                    if (ret < 1024000) m_pdu->uiMsgLen = ret;
                    if (socket->state() == QAbstractSocket::ConnectedState)
                    {
                        socket->write((char*)m_pdu, m_pdu->uiPDULen);
                        socket->waitForBytesWritten();
                    }
                }else break;
            }

            m_file.close();
            free(m_pdu);
            m_pdu = nullptr;
            usleep(1000);
            break;
        }
            // 收到下载文件回复
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            if (strcmp(pdu->caData, DOWNLOAD_FILE_OK) == 0)
            {
                qDebug() << "文件下载成功";
            }
            else if(strcmp(pdu->caData, DOWNLOAD_FILE_FAILED) == 0)
            {
                qDebug() << "文件下载失败";
            }
            emit workFinished();
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = nullptr;
    }
}

void FileTcpSocket::clientOffline()
{
    m_file.close();
    emit deleteSocket(this);
}
