#include "CSCPTest.h"
#include "./ui_CSCPTest.h"
#include <QMessageBox>
#include "tools/IPTools.h"

CSCPTest::CSCPTest(QWidget *parent) : QWidget(parent), ui(new Ui::CSCPTest) {
    ui->setupUi(this);
    newConnect = new NewConnect();
    connect(ui->bind, &QPushButton::clicked, this, &CSCPTest::bind);
    connect(ui->connectList, &QListWidget::itemSelectionChanged, this, &CSCPTest::enableOperateBtn);
    connect(ui->showMsg, &QPushButton::clicked, this, &CSCPTest::showMsg);
    connect(ui->closeConnect, &QPushButton::clicked, this, &CSCPTest::closeConnect);
    connect(ui->newConnect, &QPushButton::clicked, newConnect, &NewConnect::show);
    connect(newConnect, &NewConnect::toConnect, this, &CSCPTest::toConnect);
}

CSCPTest::~CSCPTest() {
    delete cscpManager;
    delete ui;
    delete newConnect;
}

void CSCPTest::bind() {
    auto uiCTRL = [this](bool i) {
        ui->localIP->setEnabled(!i);
        ui->localPort->setEnabled(!i);
        ui->connectList->setEnabled(i);
        ui->newConnect->setEnabled(i);
    };
    if (cscpManager == nullptr) {
        cscpManager = new CSCPManager(this);
        QByteArrayList error;
        auto ipStr = ui->localIP->text().toLocal8Bit();
        if (ipStr.isEmpty())
            error = cscpManager->bind(ui->localPort->value());
        else
            error.append(cscpManager->bind(ipStr, ui->localPort->value()));
        if (error.isEmpty()) {
            ui->bind->setText("关闭");
            uiCTRL(true);
            connect(cscpManager, &CSCPManager::connected, this, &CSCPTest::connected);
            connect(cscpManager, &CSCPManager::connectFail, this, &CSCPTest::connectFail);
            connect(cscpManager, &CSCPManager::cLog, this, &CSCPTest::appendLog);
        } else {
            QByteArray tmp;
            for (const auto &i: error)
                tmp += (i + "\n");
            QMessageBox::information(this, "绑定失败", tmp);
            delete cscpManager;
            cscpManager = nullptr;
        }
    } else {
        delete cscpManager;
        cscpManager = nullptr;
        ui->connectList->clear();
        for (auto i: connectList)
            delete i;
        connectList.clear();
        ui->bind->setText("绑定");
        uiCTRL(false);
        ui->closeConnect->setEnabled(false);
        ui->showMsg->setEnabled(false);
    }
}

void CSCPTest::closeConnect() {
    auto item = ui->connectList->currentItem();
    auto client = connectList[item->text()];
    client->getCSCP()->close();
}

void CSCPTest::enableOperateBtn() {
    ui->showMsg->setEnabled(true);
    ui->closeConnect->setEnabled(true);
}

void CSCPTest::connected(void *ptr) {
    auto cscp = (CSCP *) ptr;
    //在客户端列表里添加一个元素(IP:port)
    auto ipPort = IPPort(cscp->getIP(), cscp->getPort());
    if (!connectList.contains(ipPort)) {
        ui->connectList->addItem(ipPort);
        //去构造一个ShowMsg窗口, 以备显示
        auto sm = new ShowMsg(cscp);
        //Map保存所有客户端(ShowMsg)
        connectList.insert(ipPort, sm);
        connect(cscp, &CSCP::disconnected, this, &CSCPTest::disconnected);
    }
    {
        QByteArray IP;
        unsigned short port;
        newConnect->getTmpIPPort(IP, port);
        if (ipPort == IPPort(QHostAddress(IP), port)) {
            newConnect->restoreUI();
            newConnect->close();
        }
    }
}

void CSCPTest::showMsg() {
    auto ipPort = ui->connectList->currentItem()->text();
    connectList[ipPort]->show();
}

void CSCPTest::disconnected() {
    auto cscp = (CSCP *) sender();
    auto ipPort = IPPort(cscp->getIP(), cscp->getPort());
    //窗口
    auto client = connectList[ipPort];
    delete client;
    connectList.remove(ipPort);
    //客户端列表
    for (auto i = ui->connectList->count() - 1; i >= 0; i--) {
        auto item = ui->connectList->item(i);
        if (item->text() == ipPort) {
            ui->connectList->removeItemWidget(item);
            delete item;
            break;
        }
    }
    if (ui->connectList->count() == 0) {
        ui->closeConnect->setEnabled(false);
        ui->showMsg->setEnabled(false);
    }
}

void CSCPTest::appendLog(const QByteArray &data) {
    ui->logger->appendPlainText(data);
}

void CSCPTest::connectFail(const QHostAddress &IP, unsigned short port, const QByteArray &data) {
    newConnect->restoreUI();
    QMessageBox::information(newConnect, IPPort(IP, port) + " 连接失败", data);
}

void CSCPTest::toConnect(const QByteArray &IP, unsigned short port) {
    if (cscpManager != nullptr)
        cscpManager->createConnection(IP, port);
}
