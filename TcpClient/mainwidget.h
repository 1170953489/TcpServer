#ifndef OPEWIDGET_H
#define OPEWIDGET_H

#include <QWidget>
#include "friend.h"
#include "document.h"

namespace Ui {
class mainWidget;
}

class mainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit mainWidget(QWidget *parent = nullptr);
    ~mainWidget();
    static mainWidget &getInstance();
    Friend *getFriend();
    Document *getDocument();



private:
    Ui::mainWidget *ui;
    Friend *m_pFriend;
    Document *m_pDoc;
};

#endif // OPEWIDGET_H
