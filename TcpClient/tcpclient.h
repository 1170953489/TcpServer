#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "mainwidget.h"
#include "protocol.h"
#include "friend.h"

namespace Ui {
class TcpClient;
}

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    explicit TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();  //加载配置

    static TcpClient *getInstance();
    QTcpSocket &getTcpSocket();
    QList<FileWindow*> &getFileWindowList();
    QString loginName();
    QString curPath();
    QString curIp();
    void upProgress(qint64, qint64, int);
    void removeFileWindow(int);

public slots:
    void showConnect();
    void showDisconnect(QAbstractSocket::SocketError socketError);
    void recvMsg();

private slots:
    void on_login_pb_clicked();

    void on_regist_pb_clicked();

    void on_cancel_pb_clicked();

private:
    Ui::TcpClient *ui;

    QString m_strIP;    //存放IP
    quint16 m_usPort;   //存放端口号

    QByteArray buffer;  //缓存收到的数据
    QTcpSocket m_tcpSocket; //连接服务器，和服务器进行数据交互
    QString m_strLoginName; //当前登录用户名
    QString m_strCurPath;   //当前文件路径
    QList<FileWindow*> m_fileWindowList;//保存文件处理窗口

    bool name_ok = false;
    bool pwd_ok = false;

public:
    static int number;  //文件处理编号
};

#endif // TCPCLIENT_H
