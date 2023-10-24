#include "mytcpserver.h"
#include "filetcpserver.h"
#include <QDebug>

MyTcpSocket::MyTcpSocket()
{
    qDebug() << "MyTcpSocket生成";

    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected()), this, SLOT(clientOffline()));

    m_db = new OpeDB;
}

MyTcpSocket::~MyTcpSocket()
{
    m_db->deleteLater();
    qDebug() << "MyTcpSocket销毁";
}

QString MyTcpSocket::getStrName()
{
    return m_strName;
}

void MyTcpSocket::resendMsg(QByteArray byteArray)
{
    write(byteArray.data(), ((PDU*)(byteArray.data()))->uiPDULen);
}

void MyTcpSocket::recvMsg()
{
    buffer.append(readAll());

    // 循环解析buffer中的PDU单元
    while (buffer.size() > 0) {
        PDU *pdu = nullptr;
        uint uiPDULen = 0;
        memcpy((char*)&uiPDULen, buffer.constData(), sizeof(uint));
        if (uiPDULen > buffer.size()) break;
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
            // 收到注册请求，并回复是否成功
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);

            bool ret = m_db->handleRegist(caName, caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if (ret)
            {
                strcpy(respdu->caData, REGIST_OK);
                QDir dir;
                dir.mkdir(QString("./usr/%1").arg(caName));
            }
            else
            {
                strcpy(respdu->caData, REGIST_FAILED);
            }
            write((char*)(respdu), respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
            // 收到注销请求
        case ENUM_MSG_TYPE_CANCELL_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CANCELL_RESPOND;
            int ret = m_db->handleCancell(caName, caPwd);
            if (0 == ret)
            {
                strcpy(respdu->caData, CANCELL_OK);
                QString strPath = QString("./usr/%1").arg(caName);
                QDir dir(strPath);
                dir.removeRecursively();
            }
            else if(-1 == ret)
            {
                strcpy(respdu->caData, CANCELL_FAILED);
            }
            else if(1 == ret)
            {
                strcpy(respdu->caData, CANCELL_ONLOGIN);
            }
            write((char*)(respdu), respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            break;
        }
            // 收到登录请求，并回复是否成功
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            int ret = m_db->handleLogin(caName,caPwd);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if (0 == ret)
            {
                strcpy(respdu->caData, LOGIN_OK);
                m_strName = caName;
            }
            else if(-1 == ret)
            {
                strcpy(respdu->caData, LOGIN_FAILED);
            }
            else if(1 == ret)
            {
                strcpy(respdu->caData, RELOGIN);
            }
            write((char*)(respdu), respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            break;
        }
            // 收到获取在线用户请求，返回在线用户
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList result = m_db->handleCheckOnlineUsr();
            uint uiMsgLen = result.size()*32;
            PDU *respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for (int i = 0; i < result.size(); ++i) {
                memcpy((char*)(respdu->caMsg)+i*32, result.at(i).toStdString().c_str(), result.at(i).toStdString().size());
            }
            write((char*)(respdu), respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            break;
        }
            // 收到搜索用户请求
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            int ret = m_db->handleSerachUsr(pdu->caData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if (ret == -1)
            {
                strcpy(respdu->caData, SEARCH_USR_NO);
            }
            else if (ret == 1)
            {
                strcpy(respdu->caData, SEARCH_USR_ONLINE);
            }
            else if (ret == 0)
            {
                strcpy(respdu->caData, SEARCH_USR_OFFLINE);
            }
            write((char*)(respdu), respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            break;
        }
            // 收到添加好友请求
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData+32, 32);
            int ret = m_db->handleCheckFriend(caPerName, caName);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
            switch (ret)
            {
            case -1:
            {
                strcpy(respdu->caData, UNKNOW_ERROR);
                write((char*)(respdu), respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
                break;
            }
            case 0:
            {
                strcpy(respdu->caData, FRIEND_EXIST);
                write((char*)(respdu), respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
                break;
            }
            case 1:
            {
                MyTcpServer::getInstance().resend(caPerName, pdu);
                break;
            }
            case 2:
            {
                strcpy(respdu->caData, ADD_FRIEND_OFFLINE);
                write((char*)(respdu), respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
                break;
            }
            case 3:
            {
                strcpy(respdu->caData, ADD_FRIEND_NOTEXSIT);
                write((char*)(respdu), respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
                break;
            }
            default:
                break;
            }
            break;
        }
            // 收到同意添加好友
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData+32, 32);
            m_db->handleAddFriend(caPerName, caName);
            MyTcpServer::getInstance().resend(caName, pdu);
            break;
        }
            // 收到拒绝添加好友
        case ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData+32, 32);
            MyTcpServer::getInstance().resend(caName, pdu);
            break;
        }
            // 收到刷新好友请求
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            QStringList ret = m_db->handleFlushFriend(caName);
            PDU *respdu = mkPDU(ret.size()*32);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            for (int i=0; i<ret.size(); ++i)
            {
                memcpy((char*)(respdu->caMsg)+i*32, ret.at(i).toStdString().c_str(), 32);
            }
            write((char*)(respdu), respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            break;
        }
            // 收到删除好友请求
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            char caFriendName[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caFriendName, pdu->caData+32, 32);
            if (m_db->handleDelFriend(caName, caFriendName))
            {
                PDU *respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
                strcpy(respdu->caData, DELETE_FRIEND_OK);
                write((char*)(respdu), respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
            }
            break;
        }
            // 收到私聊请求
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caPerName[32] = {'\0'};
            memcpy(caPerName, pdu->caData+32, 32);
            MyTcpServer::getInstance().resend(caPerName, pdu);
            break;
        }
            // 收到公共聊天请求
        case ENUM_MSG_TYPE_PUBLIC_CHAT_REQUEST:
        {
            QStringList onlineUsr = m_db->handleCheckOnlineUsr();
            for (int i=0; i<onlineUsr.size(); ++i)
            {
                MyTcpServer::getInstance().resend(onlineUsr.at(i).toStdString().c_str(), pdu);
            }
            break;
        }
            // 收到创建文件夹请求
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
            bool ret = dir.exists(strCurPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;

            if (ret)    //当前目录存在
            {
                char caNewDir[64] = {'\0'};
                memcpy(caNewDir, pdu->caData, 64);
                QString strNewPath = strCurPath + "/" + caNewDir;
                ret = dir.exists(strNewPath);
                if (ret)//创建的文件夹名已存在
                {
                    strcpy(respdu->caData, FILE_NAME_EXIST);
                }
                else    //创建的文件夹名不存在，可创建
                {
                    dir.mkdir(strNewPath);
                    strcpy(respdu->caData, CREATE_DIR_OK);
                }
            }
            else        //当前目录不存在
            {
                strcpy(respdu->caData, DIR_NOT_EXIST);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;

            break;
        }
            // 收到刷新文件列表请求
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            QDir dir(pCurPath);
            QFileInfoList fileInfoLsit = dir.entryInfoList();
            int iFileCount = fileInfoLsit.size();
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;

            FileInfo *pFileInfo = nullptr;
            QString strFileName;
            for (int i=0; i<iFileCount; ++i)
            {
                pFileInfo = (FileInfo*)(respdu->caMsg)+i;
                strFileName = fileInfoLsit[i].fileName();
                memcpy(pFileInfo->caFileName, strFileName.toStdString().c_str(), strFileName.toUtf8().size());
                if (fileInfoLsit[i].isDir())
                {
                    pFileInfo->iFileType = 0;// 0表示文件夹
                }else if (fileInfoLsit[i].isFile())
                {
                    pFileInfo->iFileType = 1;// 1表示普通文件
                }
            }

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete []pCurPath;
            pCurPath = nullptr;
            break;
        }
            // 收到删除目录请求
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[64] = {'\0'};
            memcpy(caName, pdu->caData, 64);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);

            QFileInfo fileInfo(strPath);
            bool ret = false;
            if (fileInfo.isDir())
            {
                QDir dir(strPath);
                ret = dir.removeRecursively();
            }
            else
            {
                QFile file(strPath);
                ret = file.remove();
            }

            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
            if (ret)
            {
                strcpy(respdu->caData, DEL_DIR_OK);
            }
            else {
                strcpy(respdu->caData, DEL_DIR_FAILED);
            }

            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete []pPath;
            pPath = nullptr;
            break;
        }
            // 收到重命名请求
        case ENUM_MSG_TYPE_RENAME_REQUEST:
        {
            char caOldName[64] = {'\0'};
            char caNewName[64] = {'\0'};
            memcpy(caOldName, (char*)(pdu->caMsg), 64);
            memcpy(caNewName, (char*)(pdu->caMsg)+64, 64);

            char *pPath = new char[pdu->uiMsgLen-128];
            memcpy(pPath, (char*)(pdu->caMsg)+128, pdu->uiMsgLen-128);

            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

            QDir dir;
            bool ret = dir.rename(strOldPath,strNewPath);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_RENAME_RESPOND;
            if(ret)
            {
                strcpy(respdu->caData, RENAME_FILE_OK);
            }
            else
            {
                strcpy(respdu->caData, RENAME_FILE_FAILED);
            }
            write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            delete []pPath;
            pPath = nullptr;
            break;
        }
            // 收到进入文件夹请求
        case ENUM_MSG_TYPE_ENTER_DIR_REQUEST:
        {
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);

            QFileInfo fileInfo(pPath);
            if (fileInfo.isDir())
            {
                PDU *respdu = mkPDU(pdu->uiMsgLen);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caData, ENTER_DIR_OK);
                memcpy(respdu->caMsg, pdu->caMsg, pdu->uiMsgLen);

                write((char*)respdu, respdu->uiPDULen);
                free(respdu);
                respdu = nullptr;
                delete []pPath;
                pPath = nullptr;
            }
            break;
        }
        // 收到上传文件取消请求
        case ENUM_MSG_TYPE_UPLOAD_FILE_CANCEL:
        {
            int number;
            memcpy(&number, pdu->caData, sizeof(int));
            qDebug() << "取消上传文件 " << number;
            FileTcpServer::getInstance().uploadCancel(number);
            break;
        }
        // 收到下载文件取消请求
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_CANCEL:
        {
            int number;
            memcpy(&number, pdu->caData, sizeof(int));
            FileTcpServer::getInstance().downloadCancel(number);
            break;
        }

        default:
            break;
        }

        free(pdu);
        pdu = nullptr;

    }
}

void MyTcpSocket::clientOffline()
{
    m_db->handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
    emit closeThread();
}
