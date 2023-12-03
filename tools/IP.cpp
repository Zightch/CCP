#include "IP.h"
#include <QMutexLocker>
#include <QRegularExpression>
#include <QHostAddress>

namespace IPTools {
    QMutex isIPv4;
    QMutex isIPv6;
    QMutex isIPv4Port;
    QMutex isIPv6Port;
    QMutex isAnyPort;
    QMutex IPPort;
    const QRegularExpression IPv4 = QRegularExpression(R"(^((25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)\.){3}(25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)$)");
    const QRegularExpression IPv6 = QRegularExpression(R"(^(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$)");
    const QRegularExpression IPv6Port = QRegularExpression(R"(^\[(([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\.){3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))\]:([1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$)");
    const QRegularExpression IPv4Port = QRegularExpression(R"(^((25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d)\.){3}(25[0-5]|2[0-4]\d|1\d{2}|[1-9]?\d):([1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$)");
    const QRegularExpression anyPort = QRegularExpression(R"(^:([1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$)");
}

bool isIPv4(const QByteArray &data) {
    QMutexLocker ml(&IPTools::isIPv4);
    return IPTools::IPv4.match(data).hasMatch();
}

bool isIPv6(const QByteArray &data) {
    QMutexLocker ml(&IPTools::isIPv6);
    return IPTools::IPv6.match(data).hasMatch();
}

bool isIPv4Port(const QByteArray &data) {
    QMutexLocker ml(&IPTools::isIPv4Port);
    return IPTools::IPv4Port.match(data).hasMatch();
}

bool isIPv6Port(const QByteArray &data) {
    QMutexLocker ml(&IPTools::isIPv6Port);
    return IPTools::IPv6Port.match(data).hasMatch();
}

bool isAnyPort(const QByteArray &data) {
    QMutexLocker ml(&IPTools::isAnyPort);
    return IPTools::anyPort.match(data).hasMatch();
}

QByteArray IPPort(const QHostAddress &addr, unsigned short port) {
    QMutexLocker ml(&IPTools::IPPort);
    QByteArray tmp = "";
    QByteArray ip = addr.toString().toLocal8Bit();
    QByteArray portStr = QString::number(port).toLocal8Bit();
    switch (addr.protocol()) {
        case QAbstractSocket::IPv4Protocol:
            tmp = ip + ":" + portStr;
            break;
        case QAbstractSocket::IPv6Protocol:
            tmp = "[" + ip + "]:" + portStr;
            break;
        default:
            break;
    }
    return tmp;
}
