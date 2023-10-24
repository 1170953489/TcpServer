#ifndef FILETRANSFER_H
#define FILETRANSFER_H
#include <QTcpSocket>
#include <QFile>

class FileTransfer : public QTcpSocket
{
    Q_OBJECT
public:
    FileTransfer(QString, QObject *parent = nullptr);
    ~FileTransfer();
    void uploadFile();
    void downloadCancel();
    int isStop = 0;

signals:
    void workFinished();
    void upDownloadProgress(qint64, qint64);

public slots:
    void sendMsg(QByteArray);
    void recvMsg();

private:
    QString m_curFilePath;

    QByteArray buffer;      //缓存收到的数据
    QFile m_file;           //操作文件
    qint64 m_iTotal = 0;    //文件大小
    qint64 m_iRecived = 0;  //已接收文件大小
    double m_curProgress = 0;

};

#endif // FILETRANSFER_H
