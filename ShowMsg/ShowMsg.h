//
// Created by Zightch on 2023/11/5.
//

#ifndef CSCPTEST_SHOWMSG_H
#define CSCPTEST_SHOWMSG_H

#include <QWidget>
#include "CCP/CCP.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ShowMsg; }
QT_END_NAMESPACE

class ShowMsg : public QWidget {
Q_OBJECT

public:
    explicit ShowMsg(CCP *ccp, QWidget *parent = nullptr);

    ~ShowMsg() override;

    CCP *getCSCP();
private:
    Ui::ShowMsg *ui;
    CCP *ccp = nullptr;
    void recv();
    void send();
};


#endif //CSCPTEST_SHOWMSG_H
