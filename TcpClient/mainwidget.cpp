#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "tcpclient.h"
#include <QLabel>

mainWidget::mainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainWidget)
{
    ui->setupUi(this);
    m_pFriend = new Friend;
    m_pDoc = new Document;

    ui->m_pSW->insertWidget(0,m_pDoc);
    ui->m_pSW->insertWidget(1,m_pFriend);
    connect(ui->m_pListW, SIGNAL(currentRowChanged(int)), ui->m_pSW, SLOT(setCurrentIndex(int)));

    ui->m_pListW->setFocusPolicy(Qt::NoFocus);

    QWidget *widget1 = new QWidget(ui->m_pListW);
    widget1->resize(80, 80);
    widget1->setFocusPolicy(Qt::NoFocus);
    QLabel *label1 = new QLabel(widget1);
    label1->resize(80, 80);
    QPixmap pixmap1(":/icon/file.png");
    pixmap1 = pixmap1.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label1->setPixmap(pixmap1);
    label1->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout1 = new QVBoxLayout(widget1);
    layout1->addWidget(label1);
    QListWidgetItem * item1 = new QListWidgetItem(ui->m_pListW);
    item1->setFlags(item1->flags() & ~Qt::ItemIsDragEnabled);
    item1->setSizeHint(QSize(80, 80));
    ui->m_pListW->addItem(item1);
    ui->m_pListW->setItemWidget(item1, widget1);

    QWidget *widget2 = new QWidget(ui->m_pListW);
    widget2->resize(80, 80);
    widget2->setFocusPolicy(Qt::NoFocus);
    QLabel *label2 = new QLabel(widget1);
    label2->resize(80, 80);
    QPixmap pixmap2(":/icon/firend.png");
    pixmap2 = pixmap2.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    label2->setPixmap(pixmap2);
    label2->setAlignment(Qt::AlignCenter);
    QVBoxLayout *layout2 = new QVBoxLayout(widget2);
    layout2->addWidget(label2);
    QListWidgetItem * item2 = new QListWidgetItem(ui->m_pListW);
    item2->setFlags(item1->flags() & ~Qt::ItemIsDragEnabled);
    item2->setSizeHint(QSize(80, 80));
    ui->m_pListW->addItem(item2);
    ui->m_pListW->setItemWidget(item2, widget2);

    ui->m_pListW->setCurrentRow(0);
    ui->m_pSW->setCurrentIndex(0);
}

mainWidget::~mainWidget()
{
    delete m_pFriend;
    delete m_pDoc;
    delete ui;
}


mainWidget &mainWidget::getInstance()
{
    static mainWidget instance;
    instance.setWindowTitle(TcpClient::getInstance()->loginName());
    return instance;
}

Friend *mainWidget::getFriend()
{
    return m_pFriend;
}

Document *mainWidget::getDocument()
{
    return m_pDoc;
}
