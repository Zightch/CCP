#include "CSCPManager.h"
#include "tools/Dump.h"
#include "tools/IPTools.h"
#include <QDateTime>
#include <QNetworkDatagram>
#include "tools/hex.h"

CSCPManager::CSCPManager(QObject *parent) : QObject(parent) {
    connect(this, &CSCPManager::sendS_, this, &CSCPManager::sendF_, Qt::QueuedConnection);
}

void CSCPManager::proc_(const QHostAddress &IP, unsigned short port, const QByteArray &data) {
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
    if (cscp.exist(ipPort)) {
        emit cscp[ipPort]->procS_(data);
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
    if (cscp.size() >= connectNum) {
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
        auto tmp = new CSCP(this, IP, port);
        connecting[ipPort] = tmp;
        connect(tmp, &CSCP::connected_, this, &CSCPManager::connected_, Qt::QueuedConnection);
        connect(tmp, &CSCP::disconnected, this, &CSCPManager::requestInvalid_, Qt::QueuedConnection);
        emit tmp->procS_(data);
    } else {
        ef(requestReply);
        emit requestInvalid(IP, port);
    }
}

void CSCPManager::setTimeout(unsigned short t) {
    timeout = t;
}

CSCPManager::~CSCPManager() {
    close();
}

QByteArrayList CSCPManager::bind(unsigned short port) {
    QByteArrayList tmp;
    auto tmp4 = bind("0.0.0.0", port);
    if (!tmp4.isEmpty())
        tmp.append("IPv4: " + tmp4);
    auto tmp6 = bind("::", port);
    if (!tmp6.isEmpty())
        tmp.append("IPv6: " + tmp6);
    return tmp;
}

QByteArray CSCPManager::bind(const QByteArray &IP, unsigned short port) {
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
            switch (ipProtocol) {
                case 1:
                    connect(udp, &QUdpSocket::readyRead, this, &CSCPManager::recvIPv4_);
                    break;
                case 2:
                    connect(udp, &QUdpSocket::readyRead, this, &CSCPManager::recvIPv6_);
                    break;
            }
        } else {
            udpErrorInfo = udp->errorString().toLocal8Bit();
            delete udp;
            udp = nullptr;
        }
    } else
        udpErrorInfo = "CSCP管理器已绑定";
    return udpErrorInfo;
}

void CSCPManager::setConnectNum(unsigned long long cn) {
    connectNum = cn;
}

unsigned short CSCPManager::getConnectNum() const {
    return connectNum;
}

void CSCPManager::setRetryNum(unsigned char r) {
    retryNum = r;
}

void CSCPManager::sendF_(const QHostAddress& IP, unsigned short port, const QByteArray& data) {
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

void CSCPManager::connectFail_(const QByteArray& data) {
    CSCP *c = (CSCP *) sender();
    QHostAddress IP = c->IP;
    unsigned short port = c->port;
    connecting.remove(IPPort(IP, port));
    delete c;
    emit connectFail(IP, port, data);
}

void CSCPManager::connected_() {
    CSCP *c = (CSCP *) sender();
    QByteArray key = IPPort(c->IP, c->port);
    connecting.remove(key);
    if (cscp.size() < connectNum) {
        disconnect(c, &CSCP::disconnected, nullptr, nullptr);
        connect(c, &CSCP::disconnected, this, &CSCPManager::rmCSCP_);
        connect(c, &CSCP::disconnected, this, &CSCPManager::deleteCSCP_, Qt::QueuedConnection);
        cscp[key] = c;
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

void CSCPManager::deleteCSCP_() {
    delete ((CSCP *) sender());
}

void CSCPManager::close() {
    auto callBack = [this](CSCP *&i, const char *) {
        disconnect(i, &CSCP::disconnected, this, &CSCPManager::rmCSCP_);
        disconnect(i, &CSCP::disconnected, this, &CSCPManager::deleteCSCP_);
        disconnect(i, &CSCP::disconnected, this, &CSCPManager::connectFail_);
        disconnect(i, &CSCP::disconnected, this, &CSCPManager::requestInvalid_);
        i->close("管理器服务关闭");
        delete i;
        i = nullptr;
    };
    connecting.traverse(callBack);
    connecting.clear();
    cscp.traverse(callBack);
    cscp.clear();
    delete ipv4;
    ipv4 = nullptr;
    delete ipv6;
    ipv6 = nullptr;
}

int CSCPManager::isBind() {
    int tmp = 0;
    if (ipv4 != nullptr)
        tmp++;
    if (ipv6 != nullptr)
        tmp++;
    return tmp;
}

void CSCPManager::createConnection(const QByteArray &IP, unsigned short port, const QByteArray &data) {
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
    if ((cscp.size() >= connectNum)) {
        emit connectFail(ip, port, "当前管理器连接的CSCP数量已达到上限");
        return;
    }
    auto ipTmp = IPPort(QHostAddress(IP), port);
    if (cscp.exist(ipTmp)) {
        emit connected(cscp[ipTmp]);
        return;
    }
    if (!connecting.exist(ipTmp)) {
        auto tmp = new CSCP(this, QHostAddress(IP), port);
        connecting[ipTmp] = tmp;
        connect(tmp, &CSCP::connected_, this, &CSCPManager::connected_, Qt::QueuedConnection);
        connect(tmp, &CSCP::disconnected, this, &CSCPManager::connectFail_, Qt::QueuedConnection);
        tmp->initiative = true;
        tmp->connect_(data);
    }
}

unsigned short CSCPManager::getTimeout() const {
    return timeout;
}

unsigned char CSCPManager::getRetryNum() const {
    return retryNum;
}

unsigned int CSCPManager::getTotalDelay() const {
    return timeout * (retryNum + 1);
}

QByteArray CSCPManager::udpError() const {
    return udpErrorInfo;
}

void CSCPManager::rmCSCP_() {
    auto c = (CSCP *) sender();
    if (c != nullptr) {
        auto ipPort = IPPort(c->IP, c->port);
        cscp.remove(ipPort);
        connecting.remove(ipPort);
    }
}

void CSCPManager::requestInvalid_(const QByteArray&) {
    auto c = (CSCP *) sender();
    cscp.remove(IPPort(c->IP, c->port));
    emit requestInvalid(c->IP, c->port);
    delete c;
}

void CSCPManager::recvIPv4_() {
    while (ipv4->hasPendingDatagrams()) {
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

void CSCPManager::recvIPv6_() {
    while (ipv6->hasPendingDatagrams()) {
        auto datagrams = ipv6->receiveDatagram();
        auto IP = datagrams.senderAddress();
        auto port = datagrams.senderPort();
        auto data = datagrams.data();
        if (!data.isEmpty()) {
            proc_(IP, port, data);
            emit cLog("↓ " + IPPort(IP, port) + " : " + toHex(data));
        }
    }
}
