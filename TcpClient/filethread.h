#ifndef FILETHREAD_H
#define FILETHREAD_H

#include <QObject>
#include "filetransfer.h"


class FileThread : public QObject
{
    Q_OBJECT
public:
    explicit FileThread(QString, QByteArray, QObject *parent = nullptr);
    ~FileThread();
    FileTransfer *getfileTransfer();
signals:
    void sendMsg(QByteArray);
    void upProgress(qint64, qint64);
    void over();

public slots:
    void init();
    void onConnected();
    void resendProgress(qint64, qint64);
    void finish();

private:
    QString m_strLocalFilePath;
    QByteArray m_byteArray;

    FileTransfer *pFileTransfer;
};

#endif // FILETHREAD_H
