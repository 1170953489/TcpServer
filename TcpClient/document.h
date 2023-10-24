#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "protocol.h"
#include "filewindow.h"

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>


namespace Ui {
class Document;
}

class Document : public QWidget
{
    Q_OBJECT

public:
    explicit Document(QWidget *parent = nullptr);
    ~Document();
    void updateFileList(const PDU *pdu);

signals:

public slots:
    void on_m_pFlushFilePB_clicked();

private slots:
    void on_m_pCreateDirPB_clicked();

    void on_m_pRenamePB_clicked();

    void on_m_pDocLW_doubleClicked(const QModelIndex &index);

    void on_m_pDeletePB_clicked();

    void on_m_pUploadPB_clicked();

    void on_m_pDownloadPB_clicked();

    void on_ThreadFinished();


private:
    Ui::Document *ui;
    QList<FileInfo> m_fileInfo;
    QString m_strLocalFilePath;
    bool isStopUpload = false;
};

#endif // DOCUMENT_H
