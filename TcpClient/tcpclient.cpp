#include "tcpclient.h"
#include "privatechat.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QHostAddress>
#include <QRegularExpression>

int TcpClient::number = 0;

TcpClient::TcpClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpClient)
{
    loadConfig();
    // 关连信号
    connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(showConnect()));
    connect(&m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error), this, &TcpClient::showDisconnect);
    connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    // 连接服务器
    m_tcpSocket.connectToHost(QHostAddress(m_strIP), m_usPort);

    ui->setupUi(this);
    QPixmap pixmap(":/icon/yun.png");
    ui->label->setPixmap(pixmap.scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->login_pb->setEnabled(false);
    ui->login_pb->setFocusPolicy(Qt::NoFocus);
    ui->cancel_pb->setEnabled(false);
    ui->cancel_pb->setFocusPolicy(Qt::NoFocus);
    ui->regist_pb->setEnabled(false);
    ui->regist_pb->setFocusPolicy(Qt::NoFocus);

    // 控制用户名输入框内容格式
    connect(ui->name_le, &QLineEdit::textEdited, [=]() {
        QString text = ui->name_le->text();
        if (text.toStdString().size() > 30) {
            ui->name_le->setText(text.left(30));
        }
        //英文可以截断，但中文截断不了，做后续处理
        text = ui->name_le->text();
        QRegularExpression regex("[^a-zA-Z0-9\\p{Han}]");
        QRegularExpressionMatch match = regex.match(text);
        if (text.toStdString().size() > 30 || match.hasMatch()) {
            name_ok = false;
            ui->login_pb->setEnabled(false);
            ui->cancel_pb->setEnabled(false);
            ui->regist_pb->setEnabled(false);

            ui->name_le->setStyleSheet("QLineEdit{ color: rgba(250, 92, 93, 255); border: 1.5px solid rgba(250, 92, 93, 200); border-radius: 5px; background-color: rgba(255, 255, 255, 0.5);}"
                                "QLineEdit:hover{ color: rgba(250, 92, 93, 255); border: 1.5px solid rgba(250, 92, 93, 200); border-radius: 5px; background-color: rgba(255, 255, 255, 0.7);}");
        }
        else if (text.toStdString().size() <= 30 && !match.hasMatch())
        {
            if (text.toStdString().size() > 0) name_ok = true;
            else
            {
                name_ok = false;
                ui->login_pb->setEnabled(false);
                ui->cancel_pb->setEnabled(false);
                ui->regist_pb->setEnabled(false);
            }

            if (name_ok && pwd_ok)
            {
                ui->login_pb->setEnabled(true);
                ui->cancel_pb->setEnabled(true);
                ui->regist_pb->setEnabled(true);
            }
            ui->name_le->setStyleSheet("QLineEdit{ color: black; border: 1.5px solid rgba(255, 255, 255, 0); border-radius: 5px; background-color: rgba(255, 255, 255, 0.5);}"
                                "QLineEdit:hover{ color: black; border: 1.5px solid rgba(255, 255, 255, 0); border-radius: 5px; background-color: rgba(255, 255, 255, 0.7);}");
        }
    });

    // 控制密码输入框内容格式
    connect(ui->pwd_le, &QLineEdit::textEdited, [=]() {
        QString text = ui->pwd_le->text();
        if (text.toStdString().size() > 16) {
            ui->pwd_le->setText(text.left(16));
        }
        text = ui->pwd_le->text();
        QRegularExpression regex("[^a-zA-Z0-9!@#$%^&*()_+-=?.]");
        QRegularExpressionMatch match = regex.match(text);
        if (text.toStdString().size() > 16 || match.hasMatch()) {
            pwd_ok = false;
            ui->login_pb->setEnabled(false);
            ui->cancel_pb->setEnabled(false);
            ui->regist_pb->setEnabled(false);
            ui->pwd_le->setStyleSheet("QLineEdit{ color: rgba(250, 92, 93, 255); border: 1.5px solid rgba(250, 92, 93, 200); border-radius: 5px; background-color: rgba(255, 255, 255, 0.5);}"
                                "QLineEdit:hover{ color: rgba(250, 92, 93, 255); border: 1.5px solid rgba(250, 92, 93, 200); border-radius: 5px; background-color: rgba(255, 255, 255, 0.7);}");
        }
        else if (text.toStdString().size() <= 16 && !match.hasMatch())
        {
            if (text.toStdString().size() > 0) pwd_ok = true;
            else
            {
                pwd_ok = false;
                ui->login_pb->setEnabled(false);
                ui->cancel_pb->setEnabled(false);
                ui->regist_pb->setEnabled(false);
            }

            if (name_ok && pwd_ok)
            {
                ui->login_pb->setEnabled(true);
                ui->cancel_pb->setEnabled(true);
                ui->regist_pb->setEnabled(true);
            }
            ui->pwd_le->setStyleSheet("QLineEdit{ color: black; border: 1px solid rgba(255, 255, 255, 0); border-radius: 5px; background-color: rgba(255, 255, 255, 0.5);}"
                                "QLineEdit:hover{ color: black; border: 1px solid rgba(255, 255, 255, 0); border-radius: 5px; background-color: rgba(255, 255, 255, 0.7);}");
        }
    });

}

TcpClient::~TcpClient()
{
    delete ui;
}



void TcpClient::loadConfig()
{
    QFile file("config.txt");

    // 读取配置文件信息
    if (file.open(QIODevice::ReadOnly)){
        QByteArray baData = file.readAll();
        QString strData = baData.toStdString().c_str();
        file.close();

        strData.replace("\r\n", " ");   // \r\n替换为空格
        QStringList strList = strData.split(" ");   // 按空格切分
        m_strIP = strList.at(0);
        m_usPort = strList.at(1).toUShort();
        qDebug() << "ip:" << m_strIP << "port:" << m_usPort;
    }
    else {
        QMessageBox::critical(this, "open config", "open config failed");
    }
}

TcpClient *TcpClient::getInstance()
{
    static TcpClient instance;
    return &instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QList<FileWindow *> &TcpClient::getFileWindowList()
{
    return m_fileWindowList;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

QString TcpClient::curIp()
{
    return m_strIP;
}

void TcpClient::upProgress(qint64 curSize, qint64 fileSize, int number)
{
    for (int i=0; i<m_fileWindowList.size(); ++i)
    {
        if (number == m_fileWindowList.at(i)->getNumber())
        {
            m_fileWindowList.at(i)->findChild<QLabel*>("progress_LB")->setText(QString("已传输：%1/%2").arg(m_fileWindowList.at(i)->change(curSize)).arg(m_fileWindowList.at(i)->change(fileSize)));
            m_fileWindowList.at(i)->findChild<QProgressBar*>("progressBar")->setValue(double(curSize)/fileSize*1000);
            if (curSize == fileSize)
            {
                m_fileWindowList.at(i)->findChild<QPushButton*>("pushButton")->setText("完成");
                m_fileWindowList.at(i)->findChild<QLabel*>("state_LB")->setText(QString("状态：文件上传成功 . . ."));
                mainWidget::getInstance().getDocument()->on_m_pFlushFilePB_clicked();
            }
        }
    }
}

void TcpClient::removeFileWindow(int number)
{
    QList<FileWindow*>::iterator iter = m_fileWindowList.begin();
    for (; iter != m_fileWindowList.end(); ++iter)
    {
        if (number == (*iter)->getNumber())
        {
            (*iter)->deleteLater();
            *iter = nullptr;
            m_fileWindowList.erase(iter);
            qDebug() << "w已移除list";
            return;
        }
    }
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "连接服务器", "连接服务器成功");
}

void TcpClient::showDisconnect(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::ConnectionRefusedError:
        QMessageBox::information(this, "连接服务器", "未成功连接到服务器：连接被拒绝");
        break;
    case QAbstractSocket::HostNotFoundError:
        QMessageBox::information(this, "连接服务器", "未成功连接到服务器：找不到主机");
        break;
    case QAbstractSocket::SocketTimeoutError:
        QMessageBox::information(this, "连接服务器", "未成功连接到服务器：连接超时");
        break;
    default:
        QMessageBox::information(this, "连接服务器", QString("连接发生错误：'%1'").arg(m_tcpSocket.errorString()));
    }
}

// 接收服务器消息
void TcpClient::recvMsg()
{
    buffer.append(m_tcpSocket.readAll());

    // 循环解析buffer中的PDU单元
    while (buffer.size() > 0) {
        PDU *pdu = nullptr;
        int uiPDULen = 0;
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
        // 收到服务器返回注册请求结果
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if (strcmp(pdu->caData, REGIST_OK) == 0)
            {
                QMessageBox::information(this, "注册", "注册成功");
            }
            else if (strcmp(pdu->caData, REGIST_FAILED) == 0)
            {
                QMessageBox::warning(this, "注册", "注册失败：用户已存在");
            }
            break;
        }
            // 收到服务器返回注销请求
        case ENUM_MSG_TYPE_CANCELL_RESPOND:
        {
            if (strcmp(pdu->caData, CANCELL_OK) == 0)
            {
                QMessageBox::information(this, "注销", "注销成功");
                ui->name_le->clear();
                ui->pwd_le->clear();
                name_ok = false;
                pwd_ok = false;
                ui->login_pb->setEnabled(false);
                ui->cancel_pb->setEnabled(false);
                ui->regist_pb->setEnabled(false);
            }
            else if (strcmp(pdu->caData, CANCELL_ONLOGIN) == 0)
            {
                QMessageBox::warning(this, "注销", "注销失败：用户已登录，请退出登录后重试");
            }
            else if (strcmp(pdu->caData, CANCELL_FAILED) == 0)
            {
                QMessageBox::warning(this, "注销", "注销失败：请输入正确的用户名和密码");
            }
            break;
        }
        // 收到服务器返回登录请求结果
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if (strcmp(pdu->caData, LOGIN_OK) == 0)
            {
                m_strCurPath = QString("./usr/%1").arg(m_strLoginName);
                mainWidget::getInstance().show();
                this->hide();
            }
            else if (strcmp(pdu->caData, LOGIN_FAILED) == 0)
            {
                QMessageBox::warning(this, "登录", "用户名或密码错误");
            }
            else if (strcmp(pdu->caData, RELOGIN) == 0)
            {
                QMessageBox::warning(this, "登录", "用户已登录");
            }
            break;
        }
        // 收到向服务器请求在线用户的返回结果
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            mainWidget::getInstance().getFriend()->updateFriendList(pdu);
            break;
        }
        // 收到向服务器请求查找用户的返回结果
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
        {
            if (strcmp(SEARCH_USR_NO, pdu->caData) == 0)
            {
                QMessageBox::information(this, "搜索", QString("%1:不存在").arg(mainWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if (strcmp(SEARCH_USR_ONLINE, pdu->caData) == 0)
            {
                QMessageBox msgBox;
                msgBox.setWindowTitle("搜索");
                msgBox.setText(QString("%1:在线").arg(mainWidget::getInstance().getFriend()->m_strSearchName));
                msgBox.addButton(QMessageBox::Ok);
                QPushButton *addFriendButton = msgBox.addButton("加好友", QMessageBox::ActionRole);
                msgBox.exec();

                if (msgBox.clickedButton() == addFriendButton)
                {
                    QString strFriendName = mainWidget::getInstance().getFriend()->m_strSearchName;
                    QString strLoginName = loginName();
                    if (strLoginName == strFriendName)
                    {
                        QMessageBox::information(this, "添加好友", "你不能添加自己为好友");
                        return;
                    }

                    PDU *pdu = mkPDU(0);
                    pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
                    memcpy(pdu->caData, strFriendName.toStdString().c_str(), strFriendName.toStdString().size());
                    memcpy(pdu->caData+32, strLoginName.toStdString().c_str(), strLoginName.toStdString().size());
                    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
                    free(pdu);
                    pdu = nullptr;
                }
            }
            else if (strcmp(SEARCH_USR_OFFLINE, pdu->caData) == 0)
            {
                QMessageBox::information(this, "搜索", QString("%1:不在线").arg(mainWidget::getInstance().getFriend()->m_strSearchName));
            }
            break;
        }
        // 收到其他人向你添加好友的请求,返回是否同意添加好友
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData+32, 32);
            int ret = QMessageBox::information(this, "添加好友", QString("%1想添加你为好友").arg(caName), QMessageBox::Yes, QMessageBox::No);
            PDU *respdu = mkPDU(0);
            memcpy(respdu->caData, pdu->caData, 64);
            if (ret == QMessageBox::Yes)
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            }
            else
            {
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE;
            }
            m_tcpSocket.write((char*)respdu, respdu->uiPDULen);
            free(respdu);
            respdu = nullptr;
            mainWidget::getInstance().getFriend()->flushFriend();
            break;
        }
            // 收到发出添加好友请求后失败时服务器的回复
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            if (strcmp(pdu->caData, UNKNOW_ERROR) == 0)
            {
                QMessageBox::warning(this, "添加好友", "添加好友失败");
            }
            else if (strcmp(pdu->caData, FRIEND_EXIST) == 0)
            {
                QMessageBox::warning(this, "添加好友", "好友已存在");
            }
            else if (strcmp(pdu->caData, ADD_FRIEND_OFFLINE) == 0)
            {
                QMessageBox::warning(this, "添加好友", "对方不在线，暂时无法添加");
            }
            else if (strcmp(pdu->caData, ADD_FRIEND_NOTEXSIT) == 0)
            {
                QMessageBox::warning(this, "添加好友", "该用户不存在");
            }

            break;
        }
            // 对方同意添加好友的回复
        case ENUM_MSG_TYPE_ADD_FRIEND_AGREE:
        {
            char caPerName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            QMessageBox::information(this, "添加好友", QString("%1同意添加你为好友").arg(caPerName));
            mainWidget::getInstance().getFriend()->flushFriend();
            break;
        }
            // 对方拒绝添加好友的回复
        case ENUM_MSG_TYPE_ADD_FRIEND_DISAGREE:
        {
            char caPerName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            QMessageBox::information(this, "添加好友", QString("%1拒绝添加你为好友").arg(caPerName));
            break;
        }
            // 收到刷新好友列表的回复
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            mainWidget::getInstance().getFriend()->updateFriendList(pdu);
            break;
        }
            // 收到删除好友的回复
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "删除好友", "删除好友成功");
            mainWidget::getInstance().getFriend()->flushFriend();
            break;
        }
            // 收到私聊请求
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char caSendName[32] = {'\0'};
            memcpy(caSendName, pdu->caData, 32);

            QList<PrivateChat*>::iterator iter = mainWidget::getInstance().getFriend()->m_privateChatList.begin();
            for(; iter != mainWidget::getInstance().getFriend()->m_privateChatList.end(); ++iter)
            {
                if ((*iter)->getChatName() == caSendName)
                {
                    if ((*iter)->isHidden())
                    {
                        (*iter)->show();
                    }
                    (*iter)->updateMsg(pdu);
                    break;
                }
            }

            if (iter == mainWidget::getInstance().getFriend()->m_privateChatList.end())
            {
                PrivateChat *m_privateChat = new PrivateChat;
                mainWidget::getInstance().getFriend()->m_privateChatList.append(m_privateChat);
                m_privateChat->setChatName(caSendName);
                m_privateChat->show();
                m_privateChat->updateMsg(pdu);
            }
            break;
        }
            // 收到公共聊天请求
        case ENUM_MSG_TYPE_PUBLIC_CHAT_REQUEST:
        {
            mainWidget::getInstance().getFriend()->updatePublicMsg(pdu);
            break;
        }
            // 收到创建文件夹回复
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            if (strcmp(pdu->caData, CREATE_DIR_OK) == 0)
            {
//                QMessageBox::information(this, "创建文件夹", "创建成功");
                mainWidget::getInstance().getDocument()->on_m_pFlushFilePB_clicked();
            }
            else if (strcmp(pdu->caData, DIR_NOT_EXIST) == 0)
            {
                QMessageBox::warning(this, "创建文件夹", "创建失败！当前路径不存在");
            }
            else if (strcmp(pdu->caData, FILE_NAME_EXIST) == 0)
            {
                QMessageBox::warning(this, "创建文件夹", "创建失败！文件夹已存在");
            }
            break;
        }
            // 收到刷新文件回复
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            mainWidget::getInstance().getDocument()->updateFileList(pdu);
            break;
        }
            // 收到删除文件回复
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        {
            if (strcmp(pdu->caData, DEL_DIR_OK) == 0)
            {
//                QMessageBox::information(this, "删除文件", "删除成功");
                mainWidget::getInstance().getDocument()->on_m_pFlushFilePB_clicked();
            }
            else if (strcmp(pdu->caData, DEL_DIR_FAILED) == 0)
            {
                QMessageBox::warning(this, "删除文件", "删除失败");
            }
            break;
        }
            // 收到重命名回复
        case ENUM_MSG_TYPE_RENAME_RESPOND:
        {
            if (strcmp(pdu->caData, RENAME_FILE_OK) == 0)
            {
//                QMessageBox::information(this, "重命名", "重命名成功");
                mainWidget::getInstance().getDocument()->on_m_pFlushFilePB_clicked();
            }
            else if (strcmp(pdu->caData, RENAME_FILE_FAILED) == 0)
            {
                QMessageBox::warning(this, "重命名", "重命名失败");
            }
            break;
        }
            // 收到进入文件夹回复
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            if (strcmp(pdu->caData, ENTER_DIR_OK) == 0)
            {
                m_strCurPath = (char*)pdu->caMsg;
                mainWidget::getInstance().getDocument()->on_m_pFlushFilePB_clicked();
            }
            break;
        }
            // 收到上传文件进度回复
        case ENUM_MSG_TYPE_SEND_FILE_PROGRESS:
        {
            qint64 curSize, fileSize;
            int number;
            memcpy(&fileSize, pdu->caData, sizeof(qint64));
            memcpy(&curSize, pdu->caData+32, sizeof(qint64));
            memcpy(&number, pdu->caMsg, sizeof(int));
            upProgress(curSize, fileSize, number);
            break;
        }
        default:
            break;
        }
        free(pdu);
        pdu = nullptr;
    }
}

// 登录操作
void TcpClient::on_login_pb_clicked()
{
    if (m_tcpSocket.state() != QAbstractSocket::ConnectedState)
    {
        QMessageBox::critical(this, "登录", "未成功连接到服务器！");
        return;
    }
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPwd.isEmpty()) {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)(pdu), pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }else {
        return;
    }
}
// 注册请求
void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (strName.isEmpty() || strPwd.isEmpty()) return;

    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
    strncpy(pdu->caData, strName.toStdString().c_str(), 32);
    strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
    m_tcpSocket.write((char*)(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}
// 注销请求
void TcpClient::on_cancel_pb_clicked()
{
    QString strName = ui->name_le->text();
    QString strPwd = ui->pwd_le->text();
    if (strName.isEmpty() || strPwd.isEmpty()) return;

    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_CANCELL_REQUEST;
    strncpy(pdu->caData, strName.toStdString().c_str(), 32);
    strncpy(pdu->caData+32, strPwd.toStdString().c_str(), 32);
    m_tcpSocket.write((char*)(pdu), pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}
