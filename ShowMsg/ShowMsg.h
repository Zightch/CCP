#pragma once
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
    QString sendLastHexStr;
private slots:
    void recv();
    void send();
    void hex(Qt::CheckState);
    void sendDataChange();
};

