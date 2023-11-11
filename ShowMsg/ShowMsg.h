//
// Created by Zightch on 2023/11/5.
//

#ifndef CCPTEST_SHOWMSG_H
#define CCPTEST_SHOWMSG_H

#include <QWidget>
#include "CCP/CCP.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ShowMsg; }
QT_END_NAMESPACE

class ShowMsg : public QWidget {
Q_OBJECT

public:
    explicit ShowMsg(CCP *, QWidget * = nullptr);

    ~ShowMsg() override;

    CCP *getCCP();
private:
    Ui::ShowMsg *ui;
    CCP *ccp = nullptr;
    void recv();
    void send();
};


#endif //CCPTEST_SHOWMSG_H
