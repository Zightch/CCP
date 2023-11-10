#include "CCPTest.h"
#include "./ui_CCPTest.h"
#include <QMessageBox>
#include "tools/IPTools.h"

CCPTest::CCPTest(QWidget *parent) : QWidget(parent), ui(new Ui::CCPTest) {
    ui->setupUi(this);
    newConnect = new NewConnect();
    connect(ui->bind, &QPushButton::clicked, this, &CCPTest::bind);
    connect(ui->connectList, &QListWidget::itemSelectionChanged, this, &CCPTest::enableOperateBtn);
    connect(ui->showMsg, &QPushButton::clicked, this, &CCPTest::showMsg);
    connect(ui->closeConnect, &QPushButton::clicked, this, &CCPTest::closeConnect);
    connect(ui->newConnect, &QPushButton::clicked, newConnect, &NewConnect::show);
    connect(newConnect, &NewConnect::toConnect, this, &CCPTest::toConnect);
}

CCPTest::~CCPTest() {
    delete cscpManager;
    delete ui;
    delete newConnect;
}

void CCPTest::bind() {
    auto uiCTRL = [this](bool i) {
        ui->localIP->setEnabled(!i);
        ui->localPort->setEnabled(!i);
        ui->connectList->setEnabled(i);
        ui->newConnect->setEnabled(i);
    };
    if (cscpManager == nullptr) {
        cscpManager = new CCPManager(this);
        QByteArrayList error;
        auto ipStr = ui->localIP->text().toLocal8Bit();
        if (ipStr.isEmpty())
            error = cscpManager->bind(ui->localPort->value());
        else
            error.append(cscpManager->bind(ipStr, ui->localPort->value()));
        if (error.isEmpty()) {
            ui->bind->setText("关闭");
            uiCTRL(true);
            connect(cscpManager, &CCPManager::connected, this, &CCPTest::connected);
            connect(cscpManager, &CCPManager::connectFail, this, &CCPTest::connectFail);
            connect(cscpManager, &CCPManager::cLog, this, &CCPTest::appendLog);
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

void CCPTest::closeConnect() {
    auto item = ui->connectList->currentItem();
    auto client = connectList[item->text()];
    client->getCSCP()->close();
}

void CCPTest::enableOperateBtn() {
    ui->showMsg->setEnabled(true);
    ui->closeConnect->setEnabled(true);
}

void CCPTest::connected(void *ptr) {
    auto cscp = (CCP *) ptr;
    //在客户端列表里添加一个元素(IP:port)
    auto ipPort = IPPort(cscp->getIP(), cscp->getPort());
    if (!connectList.contains(ipPort)) {
        ui->connectList->addItem(ipPort);
        //去构造一个ShowMsg窗口, 以备显示
        auto sm = new ShowMsg(cscp);
        //Map保存所有客户端(ShowMsg)
        connectList.insert(ipPort, sm);
        connect(cscp, &CCP::disconnected, this, &CCPTest::disconnected);
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

void CCPTest::showMsg() {
    auto ipPort = ui->connectList->currentItem()->text();
    connectList[ipPort]->show();
}

void CCPTest::disconnected() {
    auto cscp = (CCP *) sender();
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

void CCPTest::appendLog(const QByteArray &data) {
    ui->logger->appendPlainText(data);
}

void CCPTest::connectFail(const QHostAddress &IP, unsigned short port, const QByteArray &data) {
    newConnect->restoreUI();
    QMessageBox::information(newConnect, IPPort(IP, port) + " 连接失败", data);
}

void CCPTest::toConnect(const QByteArray &IP, unsigned short port) {
    if (cscpManager != nullptr)
        cscpManager->createConnection(IP, port);
}
