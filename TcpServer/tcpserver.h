#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QWidget>
#include "mytcpserver.h"
#include "filetcpserver.h"

namespace Ui {
class TcpServer;
}

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit TcpServer(QWidget *parent = nullptr);
    ~TcpServer();

    void loadConfig();

private:
    Ui::TcpServer *ui;
    OpeDB *db;

    QString m_strIP;
    quint16 m_usPort;
};

#endif // TCPSERVER_H
