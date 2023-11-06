#include "hex.h"
#include <QByteArray>
#include <QString>
#include <QByteArrayList>

QByteArray toHex(const QByteArray &data) {
    QString hexString;
    for (char i: data)
        hexString += QString("%1 ").arg((quint8) i, 2, 16, QChar('0'));
    return hexString.toLocal8Bit();
}

QByteArray toByteArray(const QByteArray &hex) {
    auto list = hex.split(' ');
    QByteArray tmp;
    for (auto i: list) {
        if (i.isEmpty())continue;
        if (i.size() != 2)return "";
        char c;
        if ('0' <= i[0] && i[0] <= '9')c = (char) (i[0] - 48);
        else if ('A' <= i[0] && i[0] <= 'F')c = (char) (i[0] - 55);
        else if ('a' <= i[0] && i[0] <= 'f')c = (char) (i[0] - 87);
        else return "";
        c <<= 4;
        if ('0' <= i[1] && i[1] <= '9')c = (char) (c | (i[1] - 48));
        else if ('A' <= i[1] && i[1] <= 'F')c = (char) (c | (i[1] - 55));
        else if ('a' <= i[1] && i[1] <= 'f')c = (char) (c | (i[1] - 87));
        else return "";
        tmp += c;
    }
    return tmp;
}
