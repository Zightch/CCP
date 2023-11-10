#pragma once
#include <QTimer>
#include <QMap>
#include <QHostAddress>
#include <QException>

class CCPManager;

class CSCPDP {//纯数据
public:
    unsigned char cf = 0;//属性和命令
    unsigned short SID = 0;//本包ID
    QByteArray data = "";//用户数据
};

//CSCP数据包+定时器(定义)
class CDPT : public QTimer {
Q_OBJECT

public:
    explicit CDPT(QObject *);

    ~CDPT() override;

    unsigned char retryNum = 0;//重发次数
    unsigned short AID = 0;//应答包ID
    CSCPDP cdp;//数据包
};

//CSCP协议对象类(实现)
class CCP : public QObject {
Q_OBJECT

public:
    explicit CCP(QObject *, const QHostAddress &, unsigned short);

    ~CCP() override;

    void close(const QByteArray & = "");

    void sendNow(const QByteArray &);

    void send(const QByteArray &);

    [[nodiscard]]
    bool hasData() const;

    QByteArray read();

    void setDataBlockSize(unsigned short);

    void setHBTTime(unsigned short);

    [[nodiscard]]
    QHostAddress getIP() const;

    [[nodiscard]]
    unsigned short getPort() const;

public:
signals:

    void disconnected(QByteArray = "");

    void readyRead();

private:
signals:

    void procS_(QByteArray);

    void connected_();

private:
    void connect_(const QByteArray & = "");

    void procF_(QByteArray);

    void sendTimeout_();

    void sendPackage_(CDPT *);

    void NA_ACK(unsigned short AID);//应答

    void updateWnd_();//更新窗口

    CCPManager *cm = nullptr;//CCPManager
    char cs = -1;//-1未连接, 0半连接, 1连接成功, 2准备断开连接
    unsigned short ID = 0;//自己的包ID
    unsigned short OID = -1;//对方当前包ID
    QMap<unsigned short, CDPT *> sendWnd;//发送窗口
    QMap<unsigned short, CSCPDP> recvWnd;//接收窗口
    QList<CDPT *> sendBuf;//发送缓存
    QByteArrayList readBuf;//可读缓冲区
    bool link = false;
    unsigned short linkStart = 0;
    unsigned short dataBlockSize = 1024;
    QTimer hbt;//心跳包定时器
    unsigned short hbtTime = 10000;//心跳时间
    QHostAddress IP;//远程主机IP
    unsigned short port;//远程主机port
    bool initiative = false;//主动性

    friend class CCPManager;
};
