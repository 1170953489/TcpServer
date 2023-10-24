#ifndef FILEWINDOW_H
#define FILEWINDOW_H

#include <QWidget>

namespace Ui {
class FileWindow;
}

class FileWindow : public QWidget
{
    Q_OBJECT
public:
    explicit FileWindow(QString, int, QWidget *parent = nullptr);
    ~FileWindow() override;
    QString change(const qint64 &size);
    int getNumber();

private:
    void closeEvent(QCloseEvent *event) override;

signals:
    void start();   //开始
    void closed();   //未开始时关闭
    void shutdown();//中途取消
    void quit();    //结束退出

public slots:
    void downloadProgress(qint64, qint64);

private slots:
    void on_pushButton_clicked();

private:
    Ui::FileWindow *ui;
    int number = 0;
    bool closingHandled;
};

#endif // FILEWINDOW_H
