#include "CFUPTest.h"
#include "./ui_CFUPTest.h"
#include <QMessageBox>
#include "tools/tools.h"

CFUPTest::CFUPTest(QWidget *parent) : QWidget(parent), ui(new Ui::CFUPTest) {
    ui->setupUi(this);
    newConnect = new NewConnect();
    connect(ui->bind, &QPushButton::clicked, this, &CFUPTest::bind);
    connect(ui->connectList, &QListWidget::itemSelectionChanged, this, &CFUPTest::enableOperateBtn);
    connect(ui->showMsg, &QPushButton::clicked, this, &CFUPTest::showMsg);
    connect(ui->closeConnect, &QPushButton::clicked, this, &CFUPTest::closeConnect);
    connect(ui->newConnect, &QPushButton::clicked, newConnect, &NewConnect::show);
    connect(newConnect, &NewConnect::toConnect, this, &CFUPTest::toConnect);
}

CFUPTest::~CFUPTest() {
    delete ui;
}

void CFUPTest::bind() {
    auto uiCTRL = [this](bool i) {
        ui->localIP->setEnabled(!i);
        ui->localPort->setEnabled(!i);
        ui->connectList->setEnabled(i);
        ui->newConnect->setEnabled(i);
    };
    if (cfupManager == nullptr) {
        cfupManager = new CFUPManager(this);
        QStringList error;
        auto ipStr = ui->localIP->text().toLocal8Bit();
        if (ipStr.isEmpty())error = cfupManager->bind(ui->localPort->value());
        else {
            auto ret = cfupManager->bind(ipStr, ui->localPort->value());
            if (!ret.isEmpty())error.append(ret);
        }
        if (error.isEmpty()) {
            ui->bind->setText("关闭");
            uiCTRL(true);
            connect(cfupManager, &CFUPManager::connected, this, &CFUPTest::connected);
            connect(cfupManager, &CFUPManager::connectFail, this, &CFUPTest::connectFail);
            connect(cfupManager, &CFUPManager::cLog, this, &CFUPTest::appendLog);
        } else {
            QString tmp;
            for (const auto &i: error)tmp += (i + "\n");
            QMessageBox::information(this, "绑定失败", tmp.trimmed());
            cfupManager->quit();
            cfupManager = nullptr;
        }
    } else {
        cfupManager->quit();
        cfupManager = nullptr;
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

void CFUPTest::closeConnect() {
    auto item = ui->connectList->currentItem();
    auto client = connectList[item->text()];
    client->getCFUP()->close();
}

void CFUPTest::enableOperateBtn() {
    ui->showMsg->setEnabled(true);
    ui->closeConnect->setEnabled(true);
}

void CFUPTest::connected(CFUP *cfup) {
    //在客户端列表里添加一个元素(IP:port)
    auto ipPort = IPPort(cfup->getIP(), cfup->getPort());
    if (!connectList.contains(ipPort)) {
        ui->connectList->addItem(ipPort);
        //去构造一个ShowMsg窗口, 以备显示
        auto sm = new ShowMsg(cfup);
        //Map保存所有客户端(ShowMsg)
        connectList.insert(ipPort, sm);
        connect(cfup, &CFUP::disconnected, this, &CFUPTest::disconnected);
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

void CFUPTest::showMsg() {
    auto ipPort = ui->connectList->currentItem()->text();
    connectList[ipPort]->show();
}

void CFUPTest::disconnected() {
    auto cfup = (CFUP *) sender();
    auto ipPort = IPPort(cfup->getIP(), cfup->getPort());
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

void CFUPTest::appendLog(const QString &data) {
    ui->logger->appendPlainText(data);
}

void CFUPTest::connectFail(const QHostAddress &IP, unsigned short port, const QByteArray &data) {
    newConnect->restoreUI();
    QMessageBox::information(newConnect, IPPort(IP, port) + " 连接失败", data);
}

void CFUPTest::toConnect(const QByteArray &IP, unsigned short port) {
    if (cfupManager != nullptr)
        cfupManager->connectToHost(IP, port);
}

void CFUPTest::closeEvent(QCloseEvent *e) {
    if (cfupManager != nullptr)cfupManager->quit();
    cfupManager = nullptr;
    if (newConnect != nullptr) newConnect->deleteLater();
    newConnect = nullptr;
    QWidget::closeEvent(e);
}
