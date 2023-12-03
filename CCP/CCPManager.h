#pragma once
#include <QUdpSocket>
#include <QMap>
#include "CCP.h"
#include "tools/Trie.hpp"

class CCPManager : public QObject {
Q_OBJECT

public:
    explicit CCPManager(QObject * = nullptr);

    ~CCPManager() override;

    QByteArrayList bind(unsigned short);

    QByteArray bind(const QByteArray &, unsigned short);

    void setMaxConnectNum(unsigned long long);

    [[nodiscard]]
    unsigned long long getConnectNum();

    void createConnection(const QByteArray &, unsigned short, const QByteArray & = "");

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

    void sendF_(const QHostAddress &, unsigned short, const QByteArray &);

    void connectFail_(const QByteArray &);

    void connected_();

    void rmCCP_();

    void deleteCCP_();

    void requestInvalid_(const QByteArray &);

    void recv_();

    Trie<CCP *> ccp;//已连接的
    unsigned long long connectNum = 65535;//最大连接数量
    Trie<CCP *> connecting;//连接中的ccp
    QUdpSocket *ipv4 = nullptr;
    QUdpSocket *ipv6 = nullptr;
    QByteArray udpErrorInfo;

    friend class CCP;
};
