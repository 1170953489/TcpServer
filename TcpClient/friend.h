#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include "privatechat.h"

namespace Ui {
class Friend;
}

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    ~Friend();

    void updateFriendList(PDU *pdu);
    void updatePublicMsg(PDU *pdu);

    QString m_strSearchName;
    QList<PrivateChat*> m_privateChatList;

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
signals:

public slots:
    void clickOnlineButton();
    void searchUsr();
    void flushFriend();
    void delFriend();
    void privateChat();
    void publicChat();
    void addFriend();

private:
    Ui::Friend *ui;
};

#endif // FRIEND_H
