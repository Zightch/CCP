//
// Created by Zightch on 2023/11/5.
//

#ifndef CSCPTEST_SHOWMSG_H
#define CSCPTEST_SHOWMSG_H

#include <QWidget>
#include "CSCP/CSCP.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ShowMsg; }
QT_END_NAMESPACE

class ShowMsg : public QWidget {
Q_OBJECT

public:
    explicit ShowMsg(CSCP *cscp, QWidget *parent = nullptr);

    ~ShowMsg() override;

    CSCP *getCSCP();
private:
    Ui::ShowMsg *ui;
    CSCP *cscp = nullptr;
    void recv();
    void send();
};


#endif //CSCPTEST_SHOWMSG_H
