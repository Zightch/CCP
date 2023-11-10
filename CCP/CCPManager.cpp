#include "CCPManager.h"
#include "tools/Dump.h"
#include "tools/IPTools.h"
#include <QDateTime>
#include <QNetworkDatagram>
#include "tools/hex.h"

CCPManager::CCPManager(QObject *parent) : QObject(parent) {
    connect(this, &CCPManager::sendS_, this, &CCPManager::sendF_, Qt::QueuedConnection);
}

void CCPManager::proc_(const QHostAddress &IP, unsigned short port, const QByteArray &data) {
    auto ef = [&](const QByteArray &data) {
        Dump error;
        error.push((char) 0x64);
        error.push(data.data(), data.size());
        QByteArray tmp;
        tmp.append(error.get(), (qsizetype) error.size());
        emit sendS_(IP, port, tmp);
    };
    auto ipPort = IPPort(IP, port);
    if (ipPort.isEmpty()) {
        ef("IP协议不匹配");
        emit requestInvalid(IP, port);
        return;
    }
    if (ccp.exist(ipPort)) {
        emit ccp[ipPort]->procS_(data);
        return;
    }
    if (connecting.exist(ipPort)) {
        emit connecting[ipPort]->procS_(data);
        return;
    }
    const char *data_c = data.data();
    char cf = data_c[0];
    if ((cf & 0x07) != 0x01)//如果不是连接请求
        return;
    if (data.size() < 3) {
        ef("数据包不完整");
        emit requestInvalid(IP, port);
        return;
    }
    unsigned short SID = (*(unsigned short *) (data_c + 1));
    if (!((!((cf >> 5) & 0x01)) && (SID == 0))) {
        ef("数据内容不符合规范");
        emit requestInvalid(IP, port);
        return;
    }
    if (ccp.size() >= connectNum) {
        ef("当前管理器连接的CSCP数量已达到上限");
        emit requestInvalid(IP, port);
        return;
    }
    QByteArray requestReply;
    bool requestOK = true;
    if ((cf >> 6) & 0x01) {
        QByteArray rid;
        rid.append(data_c + 3, data.size() - 3);
        if (!rid.isEmpty()) emit requestInfo(IP, port, rid, requestOK, requestReply);
    }
    if (requestOK) {
        auto tmp = new CCP(this, IP, port);
        connecting[ipPort] = tmp;
        connect(tmp, &CCP::connected_, this, &CCPManager::connected_, Qt::QueuedConnection);
        connect(tmp, &CCP::disconnected, this, &CCPManager::requestInvalid_, Qt::QueuedConnection);
        emit tmp->procS_(data);
    } else {
        ef(requestReply);
        emit requestInvalid(IP, port);
    }
}

void CCPManager::setTimeout(unsigned short t) {
    timeout = t;
}

CCPManager::~CCPManager() {
    close();
}

QByteArrayList CCPManager::bind(unsigned short port) {
    QByteArrayList tmp;
    auto tmp4 = bind("0.0.0.0", port);
    if (!tmp4.isEmpty())
        tmp.append("IPv4: " + tmp4);
    auto tmp6 = bind("::", port);
    if (!tmp6.isEmpty())
        tmp.append("IPv6: " + tmp6);
    return tmp;
}

QByteArray CCPManager::bind(const QByteArray &IP, unsigned short port) {
    QHostAddress ip(IP);
    QUdpSocket **udpTmp;
    char ipProtocol = 0;
    switch (ip.protocol()) {
        case QAbstractSocket::IPv4Protocol:
            udpTmp = &ipv4;
            ipProtocol = 1;
            break;
        case QAbstractSocket::IPv6Protocol:
            udpTmp = &ipv6;
            ipProtocol = 2;
            break;
        default:
            break;
    }
    if (ipProtocol == 0) return "IP不正确";
    auto &udp = (*udpTmp);
    if (udp == nullptr) {
        udp = new QUdpSocket(this);
        if (udp->bind(ip, port)) {
            udpErrorInfo = "";
            connect(udp, &QUdpSocket::readyRead, this, &CCPManager::recv_);
        } else {
            udpErrorInfo = udp->errorString().toLocal8Bit();
            delete udp;
            udp = nullptr;
        }
    } else
        udpErrorInfo = "CSCP管理器已绑定";
    return udpErrorInfo;
}

void CCPManager::setConnectNum(unsigned long long cn) {
    connectNum = cn;
}

unsigned short CCPManager::getConnectNum() const {
    return connectNum;
}

void CCPManager::setRetryNum(unsigned char r) {
    retryNum = r;
}

void CCPManager::sendF_(const QHostAddress& IP, unsigned short port, const QByteArray& data) {
    switch (IP.protocol()) {
        case QAbstractSocket::IPv4Protocol:
            if (ipv4 != nullptr) {
                ipv4->writeDatagram(data, IP, port);
                emit cLog("↑ " + IPPort(IP, port) + " : " + toHex(data));
            } else emit cLog("IPv4未启动");
            break;
        case QAbstractSocket::IPv6Protocol:
            if (ipv6 != nullptr) {
                ipv6->writeDatagram(data, IP, port);
                emit cLog("↑ " + IPPort(IP, port) + " : " + toHex(data));
            } else emit cLog("IPv6未启动");
            break;
        default:
            break;
    }
}

void CCPManager::connectFail_(const QByteArray& data) {
    CCP *c = (CCP *) sender();
    QHostAddress IP = c->IP;
    unsigned short port = c->port;
    connecting.remove(IPPort(IP, port));
    delete c;
    emit connectFail(IP, port, data);
}

void CCPManager::connected_() {
    CCP *c = (CCP *) sender();
    QByteArray key = IPPort(c->IP, c->port);
    connecting.remove(key);
    if (ccp.size() < connectNum) {
        disconnect(c, &CCP::disconnected, nullptr, nullptr);
        connect(c, &CCP::disconnected, this, &CCPManager::rmCSCP_);
        connect(c, &CCP::disconnected, this, &CCPManager::deleteCSCP_, Qt::QueuedConnection);
        ccp[key] = c;
        emit connected(c);
    } else {
        c->close("当前连接的CSCP数量已达到上限");
        if (c->initiative) {//根据主动性触发不同的失败信号到外层
            emit connectFail(c->IP, c->port, "当前连接的CSCP数量已达到上限");
        } else {
            emit requestInvalid(c->IP, c->port);
        }
    }
}

void CCPManager::deleteCSCP_() {
    delete ((CCP *) sender());
}

void CCPManager::close() {
    auto callBack = [this](CCP *&i, const char *) {
        disconnect(i, &CCP::disconnected, this, &CCPManager::rmCSCP_);
        disconnect(i, &CCP::disconnected, this, &CCPManager::deleteCSCP_);
        disconnect(i, &CCP::disconnected, this, &CCPManager::connectFail_);
        disconnect(i, &CCP::disconnected, this, &CCPManager::requestInvalid_);
        i->close("管理器服务关闭");
        delete i;
        i = nullptr;
    };
    connecting.traverse(callBack);
    connecting.clear();
    ccp.traverse(callBack);
    ccp.clear();
    delete ipv4;
    ipv4 = nullptr;
    delete ipv6;
    ipv6 = nullptr;
}

int CCPManager::isBind() {
    int tmp = 0;
    if (ipv4 != nullptr)
        tmp++;
    if (ipv6 != nullptr)
        tmp++;
    return tmp;
}

void CCPManager::createConnection(const QByteArray &IP, unsigned short port, const QByteArray &data) {
    QHostAddress ip(IP);
    {
        QUdpSocket **udpTmp;
        char ipProtocol = 0;
        switch (ip.protocol()) {
            case QAbstractSocket::IPv4Protocol:
                udpTmp = &ipv4;
                ipProtocol = 1;
                break;
            case QAbstractSocket::IPv6Protocol:
                udpTmp = &ipv6;
                ipProtocol = 2;
                break;
            default:
                break;
        }
        if (ipProtocol == 0) {
            emit connectFail(ip, port, "IP不正确");
            return;
        }
        auto &udp = (*udpTmp);
        if (udp == nullptr) {
            emit connectFail(ip, port, "以目标IP协议所管理的CSCP管理器未启动");
            return;
        }
    }
    if ((ccp.size() >= connectNum)) {
        emit connectFail(ip, port, "当前管理器连接的CSCP数量已达到上限");
        return;
    }
    auto ipTmp = IPPort(QHostAddress(IP), port);
    if (ccp.exist(ipTmp)) {
        emit connected(ccp[ipTmp]);
        return;
    }
    if (!connecting.exist(ipTmp)) {
        auto tmp = new CCP(this, QHostAddress(IP), port);
        connecting[ipTmp] = tmp;
        connect(tmp, &CCP::connected_, this, &CCPManager::connected_, Qt::QueuedConnection);
        connect(tmp, &CCP::disconnected, this, &CCPManager::connectFail_, Qt::QueuedConnection);
        tmp->initiative = true;
        tmp->connect_(data);
    }
}

unsigned short CCPManager::getTimeout() const {
    return timeout;
}

unsigned char CCPManager::getRetryNum() const {
    return retryNum;
}

unsigned int CCPManager::getTotalDelay() const {
    return timeout * (retryNum + 1);
}

QByteArray CCPManager::udpError() const {
    return udpErrorInfo;
}

void CCPManager::rmCSCP_() {
    auto c = (CCP *) sender();
    if (c != nullptr) {
        auto ipPort = IPPort(c->IP, c->port);
        ccp.remove(ipPort);
        connecting.remove(ipPort);
    }
}

void CCPManager::requestInvalid_(const QByteArray&) {
    auto c = (CCP *) sender();
    ccp.remove(IPPort(c->IP, c->port));
    emit requestInvalid(c->IP, c->port);
    delete c;
}

void CCPManager::recv_() {
    auto udp = (QUdpSocket*)sender();
    while (udp->hasPendingDatagrams()) {
        auto datagrams = ipv4->receiveDatagram();
        auto IP = datagrams.senderAddress();
        auto port = datagrams.senderPort();
        auto data = datagrams.data();
        if (!data.isEmpty()) {
            proc_(IP, port, data);
            emit cLog("↓ " + IPPort(IP, port) + " : " + toHex(data));
        }
    }
}
