//
// Created by Zightch on 2023/11/6.
//

#ifndef CSCPTEST_NEWCONNECT_H
#define CSCPTEST_NEWCONNECT_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class NewConnect; }
QT_END_NAMESPACE

class NewConnect : public QWidget {
Q_OBJECT

public:
    explicit NewConnect(QWidget * = nullptr);

    ~NewConnect() override;

    void restoreUI();

    void getTmpIPPort(QByteArray &, unsigned short &);

public:
signals:

    void toConnect(const QByteArray &, unsigned short);

private:
    Ui::NewConnect *ui;

    void toConnect_();

    QByteArray tmpIP;
    int tmpPort = -1;
};


#endif //CSCPTEST_NEWCONNECT_H
