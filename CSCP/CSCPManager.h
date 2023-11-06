#pragma once
#include <QUdpSocket>
#include <QMap>
#include "CSCP.h"
#include "tools/Trie.hpp"

class CSCPManager : public QObject {
Q_OBJECT

public:
    explicit CSCPManager(QObject * = nullptr);

    ~CSCPManager() override;

    QByteArrayList bind(unsigned short);

    QByteArray bind(const QByteArray &, unsigned short);

    void setTimeout(unsigned short);

    void setRetryNum(unsigned char);

    void setConnectNum(unsigned long long);

    [[nodiscard]]
    unsigned short getConnectNum() const;

    void createConnection(const QByteArray &, unsigned short, const QByteArray & = "");

    [[nodiscard]]
    unsigned short getTimeout() const;

    [[nodiscard]]
    unsigned char getRetryNum() const;

    [[nodiscard]]
    unsigned int getTotalDelay() const;

    void close();

    int isBind();

    [[nodiscard]]
    QByteArray udpError() const;

public:
signals:

    void connectFail(const QHostAddress &, unsigned short, const QByteArray &);//我方主动连接连接失败

    void requestInvalid(const QHostAddress &, unsigned short);//对方请求连接连接无效

    void connected(void *);//连接成功(包含我方主动与对方请求)

    void requestInfo(const QHostAddress &, unsigned short, const QByteArray &, bool &, QByteArray &);//如果对方的连接请求包含数据, 需要外层判断数据是否合法

    void cLog(const QByteArray &);

private:
signals:

    void sendS_(const QHostAddress &, unsigned short, const QByteArray &);

private:
    void proc_(const QHostAddress &, unsigned short, const QByteArray &);

    void sendF_(const QHostAddress&, unsigned short, const QByteArray&);

    void connectFail_(const QByteArray&);

    void connected_();

    void rmCSCP_();

    void deleteCSCP_();

    void requestInvalid_(const QByteArray&);

    void recvIPv4_();

    void recvIPv6_();

    Trie<CSCP *> cscp;//已连接的

    unsigned short timeout = 1000;
    unsigned char retryNum = 2;
    unsigned long long connectNum = 65535;//最大连接数量

    Trie<CSCP *> connecting;//连接中的cscp

    QUdpSocket *ipv4 = nullptr;
    QUdpSocket *ipv6 = nullptr;
    QByteArray udpErrorInfo;

    friend class CSCP;
};
