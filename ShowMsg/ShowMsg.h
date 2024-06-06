#pragma once
#include <QWidget>
#include "CFUP/CFUP.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ShowMsg; }
QT_END_NAMESPACE

class ShowMsg : public QWidget {
Q_OBJECT

public:
    explicit ShowMsg(CFUP *, QWidget * = nullptr);

    ~ShowMsg() override;

    CFUP *getCFUP();
private:
    Ui::ShowMsg *ui;
    CFUP *cfup = nullptr;
    QString sendLastHexStr;
    QByteArrayList recvData;
private slots:
    void recv();
    void send();
    void hex(Qt::CheckState);
    void sendDataChange();
};
