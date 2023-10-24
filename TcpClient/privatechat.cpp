#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include "QMessageBox"

PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
    ui->sendMsg_pb->setEnabled(false);
    ui->sendMsg_pb->setStyleSheet("QPushButton{ color: rgba(255, 255, 255, 130); border-radius: 10px; background-color: rgba(0, 153, 255,0.5);}");

    QObject::connect(ui->inputMsg_te, &QTextEdit::textChanged, [=]() {
        if (ui->inputMsg_te->toPlainText().isEmpty()) {
            ui->sendMsg_pb->setEnabled(false);
            ui->sendMsg_pb->setStyleSheet("QPushButton{ color: rgba(255, 255, 255, 130); border-radius: 10px; background-color: rgba(0, 153, 255,0.5);}");
        } else {
            ui->sendMsg_pb->setEnabled(true);
            ui->sendMsg_pb->setStyleSheet("QPushButton{ color: rgba(255, 255, 255, 255); border-radius: 10px; background-color: rgba(0, 153, 255,1);}"
                                          "QPushButton:hover{ border-radius: 10px; background-color: rgba(0, 153, 255, 0.7);}"
                                          "QPushButton:pressed{border-radius: 10px;background-color: rgba(0, 153, 255, 1);}");
        }
    });

}

PrivateChat::~PrivateChat()
{
    delete ui;

    qDebug() << "PrivateChat 销毁";
}

void PrivateChat::setChatName(QString strChatName)
{
    m_strChatName = strChatName;
    m_strLoginName = TcpClient::getInstance()->loginName();
    setWindowTitle(QString("私聊：%1").arg(strChatName));
}

QString PrivateChat::getChatName()
{
    return m_strChatName;
}

void PrivateChat::updateMsg(const PDU *pdu)
{
    if (pdu == nullptr) return;
    char caSendName[32] = {'\0'};
    memcpy(caSendName, pdu->caData, 32);
    QString strMsg = QString("%1 :\n    %2").arg(caSendName).arg((char*)(pdu->caMsg));
    ui->showMsg_te->append(strMsg);
}

void PrivateChat::on_sendMsg_pb_clicked()
{
    QString strMsg = ui->inputMsg_te->toPlainText();
    ui->inputMsg_te->clear();
    if (!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.toStdString().size());
        pdu->uiMsgType = ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        memcpy(pdu->caData, m_strLoginName.toStdString().c_str(), m_strLoginName.toStdString().size());
        memcpy(pdu->caData+32, m_strChatName.toStdString().c_str(), m_strChatName.toStdString().size());

        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.toStdString().size());
        TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = nullptr;


        strMsg = QString("你 :\n    %1").arg(strMsg);
        ui->showMsg_te->append(strMsg);
    }
    else
    {
        QMessageBox::warning(this, "私聊", "发送的聊天信息不能为空");
    }
}
