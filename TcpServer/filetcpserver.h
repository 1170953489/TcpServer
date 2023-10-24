#ifndef FILETCPSERVER_H
#define FILETCPSERVER_H
#include <QTcpServer>
#include "filetcpsocket.h"

class FileTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    static FileTcpServer& getInstance();
    virtual void incomingConnection(qintptr socketDescriptor) override;
    void uploadCancel(int);
    void downloadCancel(int);

private slots:
    void on_ThreadFinished();
    void deleteSocket(FileTcpSocket*);

private:
    FileTcpServer();
    QList<FileTcpSocket*> m_fileSocketList;
};

#endif // FILETCPSERVER_H
