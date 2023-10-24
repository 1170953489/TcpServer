#include "filewindow.h"
#include "tcpclient.h"
#include "ui_filewindow.h"
#include <QStyle>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>

FileWindow::FileWindow(QString strTitle, int n, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FileWindow),
    closingHandled(false)
{
    ui->setupUi(this);
    this->setWindowTitle(strTitle);
    number = n;

    qDebug() << "FileWindow 生成";
}

FileWindow::~FileWindow()
{
    delete ui;
    qDebug() << "FileWindow 销毁";
}

QString FileWindow::change(const qint64 &size)
{
    int integer = 0;  //整数位
    int decimal = 0;  //小数位，保留三位
    char unit ='B';
    qint64 standardSize = size;
    qint64 curSize = size;

    if(standardSize > 1024) {
        curSize = standardSize * 1000;
        curSize /= 1024;
        integer = curSize / 1000;
        decimal = curSize % 1000;
        standardSize /= 1024;
        unit = 'K';
        if(standardSize > 1024) {
            curSize = standardSize * 1000;
            curSize /= 1024;
            integer = curSize / 1000;
            decimal = curSize % 1000;
            standardSize /= 1024;
            unit = 'M';
            if(standardSize > 1024) {
                curSize = standardSize * 1000;
                curSize /= 1024;
                integer = curSize / 1000;
                decimal = curSize % 1000;
                unit = 'G';
            }
        }
    }

    QString dec = "0";
    if (0 <= decimal && decimal <= 9) {
        dec = dec + dec + QString::number(decimal);
    }

    if (10 <= decimal && decimal <= 99) {
        dec = "0" + QString::number(decimal);
    }

    if (100 <= decimal && decimal <= 999) {
        dec = QString::number(decimal);
    }

    return QString::number(integer) + "." + dec + unit;
}

int FileWindow::getNumber()
{
    return number;
}

void FileWindow::closeEvent(QCloseEvent *event)
{
    if (!closingHandled)
    {
        if (ui->pushButton->text() == "开始")
        {
            event->accept();
            closingHandled = true;
            hide();
            emit closed();
        }
        else if (ui->pushButton->text() == "停止")
        {
            event->ignore();
            on_pushButton_clicked();
        }

        else if (ui->pushButton->text() == "完成")
        {
            event->accept();
            closingHandled = true;
            hide();
            emit quit();
        }
    }
    else
    {
        event->ignore();
    }
}

void FileWindow::on_pushButton_clicked()
{
    if (ui->pushButton->text() == "开始")
    {
        emit start();
        ui->pushButton->setText("停止");
        if (windowTitle() == "文件上传进度")
            ui->state_LB->setText(QString("状态：文件上传中 . . ."));
        else if(windowTitle() == "文件下载进度")
            ui->state_LB->setText(QString("状态：文件下载中 . . ."));
    }
    else if (ui->pushButton->text() == "停止")
    {
        if (QMessageBox::Yes == QMessageBox::information(this, "文件上传", "是否要取消上传并退出", QMessageBox::Yes, QMessageBox::No))
        {
            if (ui->pushButton->text() == "完成")
            {
                QMessageBox::information(this, "取消上传", "取消失败，文件已成功上传");
                closingHandled = true;
                hide();
                emit quit();
                return;
            }
            else
            {
                closingHandled = true;
                hide();
                emit shutdown();
                return;
            }
        }
    }
    else if (ui->pushButton->text() == "完成")
    {
        closingHandled = true;
        hide();
        emit quit();
        return;
    }
}

void FileWindow::downloadProgress(qint64 curSize, qint64 fileSize)
{
    ui->progress_LB->setText(QString("已传输：%1/%2").arg(change(curSize)).arg(change(fileSize)));
    ui->progressBar->setValue(double(curSize)/fileSize*1000);
    if (curSize == fileSize)
    {
        ui->pushButton->setText("完成");
        ui->state_LB->setText(QString("状态：文件下载成功 . . ."));
    }

}
