//
// Created by Zightch on 2023/11/5.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ShowMsg.h" resolved

#include "ShowMsg.h"
#include "ui_ShowMsg.h"

ShowMsg::ShowMsg(CSCP *cscp, QWidget *parent) : QWidget(parent), ui(new Ui::ShowMsg) {
    ui->setupUi(this);
    this->cscp = cscp;
    connect(cscp, &CSCP::readyRead, this, &ShowMsg::recv);
    connect(ui->send, &QPushButton::clicked, this, &ShowMsg::send);
}

ShowMsg::~ShowMsg() {
    delete ui;
}

CSCP *ShowMsg::getCSCP() {
    return cscp;
}

void ShowMsg::recv() {
    while(cscp->hasData()) {
        auto data  = cscp->read();
        ui->recvData->appendPlainText(data);
    }
}

void ShowMsg::send() {
    auto data = ui->sendData->toPlainText().toUtf8();
    cscp->send(data);
}
