#include "document.h"
#include "ui_document.h"
#include "tcpclient.h"
#include "filethread.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QFileDialog>
#include <QThread>


Document::Document(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Document)
{
    ui->setupUi(this);
    on_m_pFlushFilePB_clicked();
}

Document::~Document()
{
    delete ui;
}

void Document::updateFileList(const PDU *pdu)
{
    if (nullptr == pdu) return;
    for (int i=0; i<ui->m_pDocLW->count(); ++i)
    {
        QListWidgetItem *item = ui->m_pDocLW->item(i);
        if (item) {
            delete item;
        }
    }
    ui->m_pDocLW->clear();
    m_fileInfo.clear();

    FileInfo *pFileInfo = nullptr;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    for (int i=0; i<iCount; ++i)
    {
        pFileInfo = (FileInfo*)(pdu->caMsg)+i;
        if (QString(".") == pFileInfo->caFileName) continue;
        if (QString("..") == pFileInfo->caFileName
            && QString("./usr/%1").arg(TcpClient::getInstance()->loginName()) == TcpClient::getInstance()->curPath()) continue;

        m_fileInfo.append(*pFileInfo);
        QListWidgetItem *pItem = new QListWidgetItem;
        if (0 == pFileInfo->iFileType)
        {
            pItem->setIcon(QIcon(QPixmap(":/icon/dir.png")));
        }else if (1 == pFileInfo->iFileType) {
            pItem->setIcon(QIcon(QPixmap(":/icon/reg.png")));
        }
        pItem->setText(pFileInfo->caFileName);
        QFont font;
        font.setPointSize(16);
        pItem->setFont(font);

        ui->m_pDocLW->addItem(pItem);

    }
}

void Document::on_m_pCreateDirPB_clicked()
{
    QString strNewDir;
    bool ok = false;
    while (!ok)
    {
        strNewDir = QInputDialog::getText(this, "新建文件夹", "新文件夹名字：", QLineEdit::Normal, "", &ok);
        if (!ok)
        {
            return;
        }
        strNewDir = strNewDir.trimmed();

        // 检查用户输入是否为空
        if (strNewDir.isEmpty()) {
            QMessageBox::warning(this, "新建文件夹", "名字不能为空，请重新输入。");
            ok = false;
        }

        if (strNewDir == "..")
        {
            QMessageBox::critical(this, "新建文件夹", "文件夹名字不能为'..'请重新输入。");
            ok = false;
        }

        if (strNewDir.toUtf8().size() > 64)
        {
            QMessageBox::critical(this, "新建文件夹", "文件夹名字过长，请重新输入。");
            ok = false;
        }
    }

    QString strCurPath = TcpClient::getInstance()->curPath();
    PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;

    strncpy(pdu->caData, strNewDir.toStdString().c_str(), 64);
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());

    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Document::on_m_pFlushFilePB_clicked()
{
    QString strCurPath = TcpClient::getInstance()->curPath();
    ui->m_curPathLB->setText(QString("当前路径: ./%1").arg(strCurPath.mid(6)));
    PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());

    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Document::on_m_pRenamePB_clicked()
{
    QString strCurPath = TcpClient::getInstance()->curPath();
    QListWidgetItem *pItem = ui->m_pDocLW->currentItem();
    if (nullptr == pItem)
    {
        QMessageBox::critical(this, "重命名", "未选择要重命名的文件");
        return;
    }
    QString strOldName = pItem->text();

    QString strNewName;
    bool ok = false;
    while (!ok)
    {
        strNewName = QInputDialog::getText(this, "重命名", "请输入一个名字：", QLineEdit::Normal, "", &ok);
        if (!ok)
        {
            return;
        }
        strNewName = strNewName.trimmed();

        if (strNewName.isEmpty()) {
            QMessageBox::warning(this, "重命名", "名字不能为空，请重新输入。");
            ok = false;
        }
        if (strNewName == strOldName)
        {
            QMessageBox::critical(this, "重命名", "名字重复，请重新输入。");
            ok = false;
        }
        if (strNewName.toUtf8().size() > 64)
        {
            QMessageBox::critical(this, "重命名", "名字过长，请重新输入。");
            ok = false;
        }
    }

    PDU *pdu = mkPDU(strCurPath.toUtf8().size()+129);
    pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_REQUEST;
    memcpy((char*)(pdu->caMsg), strOldName.toStdString().c_str(), 64);
    memcpy((char*)(pdu->caMsg)+64, strNewName.toStdString().c_str(), 64);
    memcpy((char*)(pdu->caMsg)+128, strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());

    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Document::on_m_pDocLW_doubleClicked(const QModelIndex &index)
{
    QString strDirName = index.data().toString();
    QString strCurPath = TcpClient::getInstance()->curPath();
    QString strNewPath;
    if (strDirName == "..")
    {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index, strCurPath.size()-index);
        strNewPath = strCurPath;
    }
    else
    {
        strNewPath = QString("%1/%2").arg(strCurPath).arg(strDirName);
    }

    PDU *pdu = mkPDU(strNewPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    memcpy(pdu->caMsg, strNewPath.toStdString().c_str(), strNewPath.toUtf8().size());

    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

void Document::on_m_pDeletePB_clicked()
{
    QString strCurPath = TcpClient::getInstance()->curPath();
    QListWidgetItem *pItem = ui->m_pDocLW->currentItem();
    if (nullptr == pItem)
    {
        QMessageBox::warning(this, "删除文件", "未选择要删除的文件");
        return;
    }
    if (QMessageBox::No == QMessageBox::question(this, "删除文件", "请再次确认是否要进行删除操作", QMessageBox::Yes, QMessageBox::No)) return;

    QString strDelName = pItem->text();
    PDU *pdu = mkPDU(strCurPath.toUtf8().size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
    strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.toUtf8().size());
    strncpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.toUtf8().size());

    TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;
}

// 上传文件
void Document::on_m_pUploadPB_clicked()
{
    m_strLocalFilePath = QFileDialog::getOpenFileName();
    if (m_strLocalFilePath.isEmpty()) return;
    QFile file(m_strLocalFilePath);

    qint64 fileSize = file.size();
    int index = m_strLocalFilePath.lastIndexOf('/');
    QString strFileName = m_strLocalFilePath.right(m_strLocalFilePath.size()-index-1);
    if (strFileName.toUtf8().size() > 64)
    {
        QMessageBox::critical(this, "上传文件", "上传失败，文件名字过长。");
        return;
    }

    QString strCurPath = TcpClient::getInstance()->curPath();
    QString strNewPath = QString("%1/%2").arg(strCurPath).arg(strFileName);
    PDU *pdu = mkPDU(strNewPath.toStdString().size());
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
    sprintf(pdu->caData, "%lld", fileSize);
    int number = ++TcpClient::number;
    memcpy(pdu->caData+32, &number, sizeof(int));
    memcpy(pdu->caMsg, strNewPath.toStdString().c_str(), strNewPath.toStdString().size());
    QByteArray byteArray((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

    // 创建子线程传输文件
    QThread* thread = new QThread;
    FileThread *pFileThread = new FileThread(m_strLocalFilePath, byteArray);
    pFileThread->moveToThread(thread);

    connect(pFileThread, &FileThread::over, pFileThread, &FileThread::deleteLater);
    connect(pFileThread, &FileThread::over, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &Document::on_ThreadFinished);

    thread->start();

    // 创建文件传输窗口
    FileWindow *w = new FileWindow("文件上传进度", number);
    TcpClient::getInstance()->getFileWindowList().append(w);
    w->findChild<QLabel*>("name_LB")->setText(QString("文件名：%1").arg(strFileName));
    w->findChild<QLabel*>("progress_LB")->setText(QString("已传输：%1/%2").arg(w->change((0))).arg(w->change(fileSize)));
    w->findChild<QLabel*>("state_LB")->setText("状态：未开始上传 . . .");
    w->show();

    // 关联开始信号
    connect(w, &FileWindow::start, pFileThread, &FileThread::init);
    // 关联未开始时关闭信号
    connect(w, &FileWindow::closed, [=]() {
        QTimer::singleShot(1, pFileThread, &FileThread::finish);
        TcpClient::getInstance()->removeFileWindow(w->getNumber());
    });
    // 关联中途取消信号
    connect(w, &FileWindow::shutdown, [=]() {
        if (pFileThread->getfileTransfer()->isStop == 0)
            pFileThread->getfileTransfer()->isStop = -1;

        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_CANCEL;
        memcpy(pdu->caData, &number, sizeof(int));
        TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        TcpClient::getInstance()->getTcpSocket().flush();
        free(pdu);
        pdu = nullptr;

        pFileThread->getfileTransfer()->workFinished();
        TcpClient::getInstance()->removeFileWindow(w->getNumber());
    });
    // 关联上传成功后的关闭窗口信号
    connect(w, &FileWindow::quit, [=]() {
        TcpClient::getInstance()->removeFileWindow(w->getNumber());
    });
}

// 下载文件
void Document::on_m_pDownloadPB_clicked()
{
    QListWidgetItem *pItem = ui->m_pDocLW->currentItem();
    if (nullptr == pItem)
    {
        QMessageBox::warning(this, "下载文件", "请选择要下载的文件");
        return;
    }
    QString strFileName = pItem->text();
    for (int i=0; i<m_fileInfo.size(); ++i)
    {
        if (strFileName == m_fileInfo.at(i).caFileName && m_fileInfo.at(i).iFileType == 0)
        {
            QMessageBox::warning(this, "下载文件", "不能下载文件夹类型");
            return;
        }
    }

    QString strCurPath = TcpClient::getInstance()->curPath();
    QString strNewPath = QString("%1/%2").arg(strCurPath).arg(strFileName);
    PDU *pdu = mkPDU(strNewPath.toStdString().size());
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    int number = ++TcpClient::number;
    memcpy(pdu->caData, &number, sizeof(int));
    memcpy(pdu->caMsg, strNewPath.toStdString().c_str(), strNewPath.toStdString().size());
    QByteArray byteArray((char*)pdu, pdu->uiPDULen);
    free(pdu);
    pdu = nullptr;

    m_strLocalFilePath = QFileDialog::getSaveFileName(nullptr, QString(), strFileName);
    if (m_strLocalFilePath.isEmpty()) return;
    // 创建子线程传输文件
    QThread* thread = new QThread;
    FileThread *pFileThread = new FileThread(m_strLocalFilePath, byteArray);
    pFileThread->moveToThread(thread);

    connect(pFileThread, &FileThread::over, pFileThread, &FileThread::deleteLater);
    connect(pFileThread, &FileThread::over, thread, &QThread::quit);
    connect(thread, &QThread::finished, this, &Document::on_ThreadFinished);

    thread->start();

    // 创建文件传输窗口
    FileWindow *w = new FileWindow("文件下载进度", number);
    TcpClient::getInstance()->getFileWindowList().append(w);
    w->findChild<QLabel*>("name_LB")->setText(QString("文件名：%1").arg(strFileName));
    w->findChild<QLabel*>("progress_LB")->setText(QString("已传输：%1/%2").arg(w->change((0))).arg(w->change(0)));
    w->findChild<QLabel*>("state_LB")->setText("状态：未开始下载 . . .");
    w->show();

    // 关联开始信号
    connect(w, &FileWindow::start, pFileThread, &FileThread::init);
    // 关联未开始时关闭信号
    connect(w, &FileWindow::closed, [=]() {
        QTimer::singleShot(1, pFileThread, &FileThread::finish);
        TcpClient::getInstance()->removeFileWindow(w->getNumber());
    });
    // 关联中途取消信号
    connect(w, &FileWindow::shutdown, [=]() {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_CANCEL;
        memcpy(pdu->caData, &number, sizeof(int));
        TcpClient::getInstance()->getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        TcpClient::getInstance()->getTcpSocket().flush();
        free(pdu);
        pdu = nullptr;

        pFileThread->getfileTransfer()->downloadCancel();
        pFileThread->getfileTransfer()->workFinished();
        TcpClient::getInstance()->removeFileWindow(w->getNumber());
    });
    // 关联下载成功后的关闭信号
    connect(w, &FileWindow::quit, [=]() {
        TcpClient::getInstance()->removeFileWindow(w->getNumber());
    });
    // 关联更新进度信号
    connect(pFileThread, &FileThread::upProgress, w, &FileWindow::downloadProgress);
}

void Document::on_ThreadFinished()
{
    qDebug() << "Thread has finished!";
}

