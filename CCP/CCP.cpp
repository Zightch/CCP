#include "CCP.h"
#include "CCPManager.h"
#include "tools/Dump.h"
#include <QDateTime>

CCP::CCP(QObject *parent, const QHostAddress&IP, unsigned short p) : QObject(parent), IP(IP), port(p) {
    cm = ((CCPManager *) parent);
    connect(this, &CCP::procS_, this, &CCP::procF_, Qt::QueuedConnection);
    connect(&hbt, &QTimer::timeout, this, [&]() {
        if (cs == 1) {
            auto *cdpt = new CDPT(this);
            cdpt->cf = 0x05;
            cdpt->SID = ID;
            sendBuf.append(cdpt);
            updateWnd_();
        }
    });
}

void CCP::close(const QByteArray &data) {
    if (cs == 1) {
        auto *cdpt = new CDPT(this);
        cdpt->cf = 0x24;
        cdpt->SID = ID;
        if (!data.isEmpty()) {
            cdpt->cf |= 0x40;
            cdpt->data = data;
        }
        sendPackage_(cdpt);
        cs = -1;
        emit disconnected(data);
    }
    readBuf.append(data);
    for (auto i: sendWnd)
        delete i;
    for (auto i: sendBuf)
        delete i;
    sendWnd.clear();
    sendBuf.clear();
    recvWnd.clear();
    hbt.stop();
}

void CCP::procF_(QByteArray data) {
    const char *data_c = data.data();
    unsigned char cf = data_c[0];

    //bool UDL = ((cf >> 7) & 0x01);
    bool UD = ((cf >> 6) & 0x01);
    bool NA = ((cf >> 5) & 0x01);
    bool RT = ((cf >> 4) & 0x01);
    auto cmd = (unsigned char) (cf & (unsigned char) 0x07);

    if (!(NA && RT)) {
        if (1 <= cmd && cmd <= 5) {
            switch (cmd) {
                case 1: {
                    auto *tmp = new CDPT(this);
                    tmp->SID = 0;
                    tmp->AID = 0;
                    tmp->cf = (char) 0x03;
                    sendBuf.append(tmp);
                    if (cs == -1) {
                        OID = 0;
                        cs = 0;//半连接
                    }
                    break;
                }
                case 2: {
                    if (NA) {
                        unsigned short AID = (*(unsigned short *) (data_c + 1));
                        if (sendWnd.count(AID) == 1) {
                            sendWnd[AID]->stop();
                            if (AID == 0 && cs == 0) {
                                cs = 1;
                                emit connected_();
                            }
                        }
                    }
                    break;
                }
                case 3: {
                    if (ID == 0 && OID == 65535 && !RT) {
                        NA_ACK(0);
                        unsigned short SID = (*(unsigned short *) (data_c + 1));
                        unsigned short AID = (*(unsigned short *) (data_c + 3));
                        if (cs == 0 && SID == 0 && AID == 0) {
                            ID = 1;
                            OID = 0;
                            cs = 1;
                            delete sendWnd[0];
                            sendWnd.remove(0);
                            emit connected_();
                            hbt.start(hbtTime);
                        }
                    } else if (RT)
                        NA_ACK(0);
                    break;
                }
                case 4: {
                    if (NA) {//紧急断开
                        QByteArray userData;
                        if (UD)
                            userData.append(data_c + 1, data.size() - 1);
                        cs = -1;
                        readBuf.append(userData);
                        for (auto i: sendWnd)
                            delete i;
                        for (auto i: sendBuf)
                            delete i;
                        sendWnd.clear();
                        sendBuf.clear();
                        recvWnd.clear();
                        hbt.stop();
                        emit disconnected(userData);
                    }
                    break;
                }
                case 5: {
                    if (cs == 1) {
                        unsigned short SID = (*(unsigned short *) (data_c + 1));
                        NA_ACK(SID);
                        if (SID == OID + 1) {
                            OID = SID;
                            hbt.stop();
                            hbt.start(hbtTime);
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        } else {
            if (!NA) {//需要回复
                unsigned short SID = (*(unsigned short *) (data_c + 1));
                NA_ACK(SID);
                if (UD) {//有用户数据
                    //创建一个字节数组来存储用户数据
                    QByteArray userData;
                    //从数据包中提取用户数据，跳过前三个字节的头部信息
                    userData.append(data_c + 3, data.size() - 3);
                    //如果是重发包，并且接收窗口中已经有该数据，则不需要再次存储
                    if (!RT || !recvWnd.contains(SID)) (recvWnd[SID] = {cf, SID, userData});
                }
            } else if (UD) {//有用户数据
                QByteArray userData;
                userData.append(data_c + 1, data.size() - 1);
                readBuf.append(userData);
                emit readyRead();
            }
        }
    }
    updateWnd_();
}

void CCP::sendNow(const QByteArray &data) {
    if (cs == 1) {
        auto *tmp = new CDPT(this);
        tmp->data = data;
        tmp->cf = 0x60;
        sendBuf.append(tmp);
        updateWnd_();
    }
}

void CCP::send(const QByteArray &data) {
    if (cs == 1) {
        if (data.size() <= dataBlockSize) {
            auto *tmp = new CDPT(this);
            tmp->data = data;
            tmp->cf = 0x40;
            tmp->SID = ID;
            sendBuf.append(tmp);
        } else {
            QByteArrayList dataBlock;
            QByteArray i = data, tmp;
            while (!i.isEmpty()) {
                unsigned short dbs = dataBlockSize;
                if (i.size() <= dataBlockSize)
                    dbs = i.size();
                tmp.append(i, dbs);
                dataBlock.append(tmp);
                tmp.clear();
                tmp = i;
                i.clear();
                i.append(tmp.data() + dbs, tmp.size() - dbs);
                tmp.clear();
            }
            if (dataBlock.size() <= 65534) {
                for (auto j = 0; j < dataBlock.size(); j++) {
                    auto *cdpt = new CDPT(this);
                    cdpt->data = dataBlock[(qsizetype) j];
                    cdpt->SID = ID + j;
                    if (j != dataBlock.size() - 1)
                        cdpt->cf = 0xC0;
                    else
                        cdpt->cf = 0x40;
                    sendBuf.append(cdpt);
                }
            } else
                throw "数据内容超过最大连续发送大小";
        }
        updateWnd_();
    }
}

bool CCP::hasData() const {
    return !readBuf.empty();
}

QByteArray CCP::read() {
    QByteArray tmp = readBuf.first();
    readBuf.pop_front();
    return tmp;
}

void CCP::setDataBlockSize(unsigned short us) {
    if (us >= 65530)
        dataBlockSize = 65530;
    else dataBlockSize = us;
}

void CCP::setHBTTime(unsigned short time) {
    hbtTime = time;
    if (cs == 1) {
        hbt.stop();
        hbt.start(hbtTime);
    }
}

QHostAddress CCP::getIP() const {
    return IP;
}

unsigned short CCP::getPort() const {
    return port;
}

void CCP::connect_(const QByteArray &data) {
    if (cs == -1) {
        auto *tmp = new CDPT(this);
        tmp->SID = 0;
        tmp->cf = (char) 0x01;
        if (data.size() != 0) {
            tmp->cf |= (char) 0x40;
            tmp->data = data;
        }
        sendBuf.append(tmp);
        cs = 0;//半连接
        updateWnd_();
    }
}

CCP::~CCP() {
    close();
    cm = nullptr;
    readBuf.clear();
}

void CCP::sendTimeout_() {
    auto *cdpt = (CDPT *) sender();
    if (cdpt->retryNum < cm->getRetryNum()) {
        cdpt->retryNum++;
        cdpt->cf |= 0x10;
        sendWnd.remove(cdpt->SID);
        sendBuf.append(cdpt);
    } else {
        cs = -1;
        readBuf.append("对方应答超时");
        for (auto i: sendWnd)
            delete i;
        for (auto i: sendBuf)
            delete i;
        sendBuf.clear();
        sendWnd.clear();
        recvWnd.clear();
        hbt.stop();
        emit disconnected("对方应答超时");
    }
    updateWnd_();
}

void CCP::sendPackage_(CDPT *cdpt) {
    Dump d;
    d.push(cdpt->cf);
    unsigned char cmd = (char) (cdpt->cf & (char) 0x07);
    bool NA = (cdpt->cf >> 5) & 0x01;
    if (!NA)
        d.push(cdpt->SID);
    if ((cmd == 2) || (cmd == 3))
        d.push(cdpt->AID);
    if ((cdpt->cf >> 6) & 0x01)
        d.push(cdpt->data, cdpt->data.size());
    QByteArray tmp;
    tmp.append(d.get(), (qsizetype) d.size());
    cm->sendF_(IP, port, tmp);
    if (!NA) {
        disconnect(cdpt, &CDPT::timeout, this, &CCP::sendTimeout_);
        connect(cdpt, &CDPT::timeout, this, &CCP::sendTimeout_);
        cdpt->start(cm->getTimeout());
        sendWnd[cdpt->SID] = cdpt;
        hbt.stop();
    } else
        delete cdpt;
}

void CCP::NA_ACK(unsigned short AID) {
    auto *tmp = new CDPT(this);
    tmp->AID = AID;
    tmp->cf = (char) 0x22;
    sendBuf.append(tmp);
    updateWnd_();
}

void CCP::updateWnd_() {
    while (sendWnd.contains(ID)) {
        if (!sendWnd[ID]->isActive()) {
            delete sendWnd[ID];
            sendWnd.remove(ID);
            ID++;
        } else
            break;
    }
    if ((sendWnd.size() < 256) && (!sendBuf.isEmpty())) {
        sendPackage_(sendBuf.front());
        sendBuf.pop_front();
    }
    if (sendWnd.isEmpty() && (!hbt.isActive()))
        hbt.start(hbtTime);
    //接收窗口连续性判断
    bool isRead = false;
    while (recvWnd.contains(OID + 1)) {
        if ((((recvWnd[OID + 1].cf) >> 7) & 0x01)) {//如果是链表包,转为链表状态
            if (!link) {
                link = true;
                linkStart = OID + 1;
            }
        } else {
            if (link) {
                //合并包并触发readyRead
                QByteArray dataFull;
                unsigned short j = linkStart;
                while (j != OID + 2) {
                    dataFull.append(recvWnd[j].data, recvWnd[j].data.size());
                    recvWnd.remove(j);
                    j++;
                }
                readBuf.append(dataFull);
                link = false;
            } else {
                readBuf.append(recvWnd[OID + 1].data);
                recvWnd.remove(OID + 1);
            }
            isRead = true;
        }
        OID++;
    }
    if (isRead) emit readyRead();
}

CDPT::CDPT(QObject *parent) : QTimer(parent){

}

CDPT::~CDPT() = default;
