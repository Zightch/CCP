#pragma once

class QByteArray;
class QHostAddress;

bool isIPv4(const QByteArray &);

bool isIPv6(const QByteArray &);

bool isIPv4Port(const QByteArray &);

bool isIPv6Port(const QByteArray &);

bool isAnyPort(const QByteArray &);

QByteArray IPPort(const QHostAddress &, unsigned short);
