#include "friend.h"
#include "ui_friend.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox>

Friend::Friend(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Friend)
{
    ui->setupUi(this);
    flushFriend();

    connect(ui->m_pShowOnlineUsrPB, SIGNAL(clicked(bool)), this, SLOT(clickOnlineButton()));
    connect(ui->m_pSearchUsrPB, SIGNAL(clicked(bool)), this, SLOT(searchUsr()));
    connect(ui->m_pFlushFriendPB, SIGNAL(clicked(bool)), this, SLOT(flushFriend()));
    connect(ui->m_pDelFriendPB, SIGNAL(clicked(bool)), this, SLOT(delFriend()));
    connect(ui->m_pPrivateChatPB, SIGNAL(clicked(bool)), this, SLOT(privateChat()));
    connect(ui->m_pMsgSendPB, SIGNAL(clicked(bool)), this, SLOT(publicChat()));
    connect(ui->m_pAddFriend_PB, SIGNAL(clicked(bool)), this, SLOT(addFriend()));

    ui->m_pMsgSendPB->setEnabled(false);
    ui->m_pMsgSendPB->setStyleSheet("QPushButton{ color: rgba(255, 255, 255, 130); border-radius: 10px; background-color: rgba(0, 153, 255,0.5);}");
    connect(ui->m_pInputMsgTE, &QTextEdit::textChanged, [=]() {
        if (ui->m_pInputMsgTE->toPlainText().isEmpty()) {
            ui->m_pMsgSendPB->setEnabled(false);
            ui->m_pMsgSendPB->setStyleSheet("QPushButton{ color: rgba(255, 255, 255, 130); border-radius: 10px; background-color: rgba(0, 153, 255,0.5);}");
        } else {
            ui->m_pMsgSendPB->setEnabled(true);
            ui->m_pMsgSendPB->setStyleSheet("QPushButton{ color: rgba(255, 255, 255, 255); border-radius: 10px; background-color: rgba(0, 153, 255,1);}"
                                          "QPushButton:hover{ border-radius: 10px; background-color: rgba(0, 153, 255, 0.7);}"
                                          "QPushButton:pressed{border-radius: 10px;background-color: rgba(0, 153, 255, 1);}");
        }
    });
    ui->m_pInputMsgTE->installEventFilter(this);
}

Friend::~Friend()
{
    delete ui;
}

bool Friend::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->m_pInputMsgTE && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (ui->m_pMsgSendPB->isEnabled()) ui->m_pMsgSendPB->clicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void Friend::updateFriendList(PDU *pdu)
{
    if (pdu == nullptr)
    {
        return;
    }
    ui->m_pFriendListWidget->clear();
    uint uiSize = pdu->uiMsgLen/32;
    char caName[32] = {'\0'};
    for (uint i=0; i<uiSize; ++i)
    {
        memcpy(caName, (char*)(pdu->caMsg)+i*32, 32);
        ui->m_pFriendListWidget->addItem(caName);
    }
}

void Friend::updatePublicMsg(PDU *pdu)
{
    QString strMsg = QString("%1 :\n    %2").arg(pdu->caData).arg((char*)(pdu->caMsg));
    ui->m_pShowMsgTE->append(strMsg);
}

void Friend::clickOnlineButton()
{
    ui->m_usrListLB->setText("在线用户：");
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::searchUsr()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名:");
    if (!m_strSearchName.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_REQUEST;
        strncpy(pdu->caData, m_strSearchName.toStdString().c_str(), m_strSearchName.toStdString().size());
        TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Friend::flushFriend()
{
    ui->m_usrListLB->setText("好友：");
    QString strName = TcpClient::getInstance()->loginName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    strncpy(pdu->caData, strName.toStdString().c_str(),strName.toStdString().size());
    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Friend::delFriend()
{
    if (ui->m_usrListLB->text() == "在线用户：") return;
    if (ui->m_pFriendListWidget->currentItem() != nullptr)
    {
        QString strName = TcpClient::getInstance()->loginName();
        QString strFriendName = ui->m_pFriendListWidget->currentItem()->text();

        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        memcpy(pdu->caData, strName.toStdString().c_str(), strName.toStdString().size());
        memcpy(pdu->caData+32, strFriendName.toStdString().c_str(), strFriendName.toStdString().size());
        TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;
    }
}

void Friend::privateChat()
{
    if (ui->m_usrListLB->text() == "在线用户：") return;
    if (ui->m_pFriendListWidget->currentItem() != nullptr)
    {
        QString strChatName = ui->m_pFriendListWidget->currentItem()->text();
        QList<PrivateChat*>::iterator iter = m_privateChatList.begin();
        for(; iter != m_privateChatList.end(); ++iter)
        {
            if ((*iter)->getChatName() == strChatName)
            {
                if ((*iter)->isHidden())
                {
                    (*iter)->show();
                }
                break;
            }
        }

        if (iter == m_privateChatList.end())
        {
            PrivateChat *m_privateChat = new PrivateChat;
            m_privateChatList.append(m_privateChat);
            m_privateChat->setChatName(strChatName);
            m_privateChat->show();
        }
    }
    else
    {
        QMessageBox::warning(this, "私聊", "请选择要私聊的好友");
    }
}

void Friend::publicChat()
{
    QString strMsg = ui->m_pInputMsgTE->toPlainText();
    if (strMsg.isEmpty())
    {
        QMessageBox::critical(this, "发送", "发送信息不能为空");
        return;
    }

    ui->m_pInputMsgTE->clear();
    PDU *pdu = mkPDU(strMsg.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_PUBLIC_CHAT_REQUEST;
    QString strName = TcpClient::getInstance()->loginName();
    strncpy(pdu->caData, strName.toStdString().c_str(), strName.toStdString().size());
    strcpy((char*)(pdu->caMsg), strMsg.toStdString().c_str());

    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
}

void Friend::addFriend()
{
    if (ui->m_usrListLB->text() == "好友：") return;
    QListWidgetItem *pItem = ui->m_pFriendListWidget->currentItem();
    if (!pItem) return;
    QString strFriendName = pItem->text();
    QString strLoginName = TcpClient::getInstance()->loginName();
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

