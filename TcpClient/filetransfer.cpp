#include "filetransfer.h"
#include "tcpclient.h"
#include <QMessageBox>

FileTransfer::FileTransfer(QString curPath, QObject *parent) : QTcpSocket(parent)
{
    m_curFilePath = curPath;
    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));

    qDebug() << "FileTransfer 生成";
}

FileTransfer::~FileTransfer()
{
    qDebug() << "FileTransfer 销毁";
}

// 发送上传和下载的请求PDU
void FileTransfer::sendMsg(QByteArray byteArray)
{
    write(byteArray.data(), ((PDU*)(byteArray.data()))->uiPDULen);
    flush();
    if (((PDU*)(byteArray.data()))->uiMsgType == ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST)
    {
        sleep(1);
        uploadFile();
    }
}

// 上传文件
void FileTransfer::uploadFile()
{
    m_file.setFileName(m_curFilePath);
    if (!m_file.open(QIODevice::ReadOnly)) return;

    PDU *pdu = mkPDU(2000000);
    pdu->uiMsgType = ENUM_MSG_TYPE_SEND_FILE;
    uint ret = 0;
    qint64 total = 0;
    while (true)
    {
        if (isStop) //中途停止
        {


            qDebug() << "中途停止上传";
            this->close();
            break;
        }
        ret = m_file.read((char*)(pdu->caMsg), 2000000);
        if (ret > 0 && ret <= 2000000) //循环上传部分
        {
            if (ret < 2000000) pdu->uiMsgLen = ret;
            write((char*)pdu, pdu->uiPDULen);
            qDebug() << (total = total+ret+76) << bytesToWrite();
            waitForBytesWritten();
        }
        else //结束上传
        {
            flush();
            isStop = 1;
            break;
        }
    }

    m_file.close();
    free(pdu);
    pdu = nullptr;
    usleep(1000);
}

void FileTransfer::downloadCancel()
{
    m_file.close();
    m_file.remove();
    emit workFinished();
}

// 接收消息
void FileTransfer::recvMsg()
{
    buffer.append(readAll());
    while (buffer.size() > 0)
    {
        PDU *pdu = nullptr;
        int uiPDULen = 0;
        memcpy((char*)&uiPDULen, buffer.constData(), sizeof(uint));
        if (uiPDULen > buffer.size())
        {
            break;
        }
        else
        {
            uint uiMsgLen = uiPDULen - sizeof (PDU);
            pdu = mkPDU(uiMsgLen);
            memcpy((char*)pdu, buffer.constData(), uiPDULen);
            buffer.remove(0, uiPDULen);
        }

        // 判断数据类型
        switch (pdu->uiMsgType)
        {
        // 收到上传文件回复
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            if (strcmp(pdu->caData, UPLOAD_FILE_OK) == 0)
            {
                qDebug() << "文件上传成功";
            }
            else if (strcmp(pdu->caData, UPLOAD_FILE_FAILED) == 0)
            {
                qDebug() << "文件上传失败";
            }
            emit workFinished();
            break;
        }
            // 写入下载的文件
        case ENUM_MSG_TYPE_SEND_FILE:
        {
            if (isStop == -1) break;
            if (!m_file.isOpen())
            {
                m_file.setFileName(m_curFilePath);
                if (m_file.open(QIODevice::WriteOnly))
                {
                    memcpy(&m_iTotal, pdu->caData, sizeof(qint64));
                    emit upDownloadProgress(m_iRecived, m_iTotal);
                }
            }
            //-----------------------------------------------------------
            QByteArray buff(pdu->caMsg, pdu->uiMsgLen);
            m_file.write(buff);
            m_iRecived += buff.size();
            //-----------------------------------------------------------
            if (double(m_iRecived)/m_iTotal*100 - m_curProgress >= 0.01 || m_iTotal == m_iRecived)
            {
                m_curProgress = double(m_iRecived)/m_iTotal*100;
                emit upDownloadProgress(m_iRecived, m_iTotal);
            }
            if (m_iTotal == m_iRecived)
            {
                m_file.close();

                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
                strcpy(respdu->caData, DOWNLOAD_FILE_OK);
                write((char*)(respdu), respdu->uiPDULen);
                flush();
                free(respdu);
                respdu = nullptr;

                waitForBytesWritten();
                close();

                emit workFinished();
                break;
            }
            else if (m_iTotal < m_iRecived)
            {
                m_file.remove();
                m_file.close();

                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
                strcpy(respdu->caData, DOWNLOAD_FILE_FAILED);
                write((char*)(respdu), respdu->uiPDULen);
                flush();
                free(respdu);
                respdu = nullptr;

                waitForBytesWritten();
                close();

                emit workFinished();
                break;
            }
            break;
        }
        }
        free(pdu);
        pdu = nullptr;
    }
}
