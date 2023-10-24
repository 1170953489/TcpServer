#include "tcpserver.h"
#include "ui_tcpserver.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QFile>

TcpServer::TcpServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpServer),
    db(new OpeDB)
{
    ui->setupUi(this);
    db->init();
    QDir dir("./usr");
    if (!dir.exists()) dir.mkpath("../usr");

    loadConfig();
    MyTcpServer::getInstance().listen(QHostAddress::Any, m_usPort);
    FileTcpServer::getInstance().listen(QHostAddress::Any, 9999);
}

TcpServer::~TcpServer()
{
    delete ui;
    db->handleServerOffline();
    db->deleteLater();
}

void TcpServer::loadConfig()
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
