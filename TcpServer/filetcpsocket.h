#ifndef FILETCPSOCKET_H
#define FILETCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"

#include <QDir>
#include <QFile>


class FileTcpSocket : public QObject
{
    Q_OBJECT
public:
    FileTcpSocket();
    ~FileTcpSocket();
    int getNumber();
    QString getUsrName();
    void uploadCancel();
    bool isStop;
    double m_curProgress = 0;  //当前进度

signals:
    void sendProgress(qint64, qint64);
    void workFinished();
    void closeThread();
    void deleteSocket(FileTcpSocket*);

public slots:
    void init(qintptr);
    void recvMsg();
    void clientOffline();

private:
    QTcpSocket *socket;
    QString m_usrName;

    QByteArray buffer;      //缓存收到的数据
    QFile m_file;           //操作文件
    int number = 0;         //记录文件编号
    qint64 m_iTotal = 0;    //文件大小
    qint64 m_iRecived = 0;  //已接收文件大小
};

#endif // FILETCPSOCKET_H
