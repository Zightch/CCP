#include "ShowMsg.h"
#include "ui_ShowMsg.h"
#include "tools/tools.h"

ShowMsg::ShowMsg(CCP *ccp, QWidget *parent) : QWidget(parent), ui(new Ui::ShowMsg) {
    ui->setupUi(this);
    this->ccp = ccp;
    setWindowTitle(IPPort(ccp->getIP(), ccp->getPort()));
    connect(ccp, &CCP::readyRead, this, &ShowMsg::recv);
    connect(ui->send, &QPushButton::clicked, this, &ShowMsg::send);
}

ShowMsg::~ShowMsg() {
    delete ui;
}

CCP *ShowMsg::getCCP() {
    return ccp;
}

void ShowMsg::recv() {
    while(ccp->hasData()) {
        auto data  = ccp->nextPendingData();
        ui->recvData->appendPlainText(data);
    }
}

void ShowMsg::send() {
    auto data = ui->sendData->toPlainText().toUtf8();
    ccp->send(data);
}
