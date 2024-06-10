#include "CFUP.h"
#include "CFUPManager.h"

void CFUP::cmdRC_(const QByteArray &data) { // 已经被CFUPManager过滤过了, 不用二次判断
    if (cs != -1 || initiative)return; // 连接状态: 未连接, 而且不能是主动连接
    long long time = *(long long *) (data.data() + 3);
    if (!time_(0, time))return; // 时间不正确
    auto cdpt = newCDPT_(); // 构建回复数据包
    cdpt->SID = 0;
    cdpt->AID = 0;
    cdpt->cf = 0x03;
    sendBufLv1.append(cdpt);
    OID = 0;
    cs = 0; // 半连接
}

void CFUP::cmdACK_(bool NA, const QByteArray &data) {
    if (!NA) return;
    if (data.size() != 3)return;
    unsigned short AID = (*(unsigned short *) (data.data() + 1));
    if (cs == 0) { // 如果是半连接状态
        if (AID == 0 && !initiative) {
            sendWnd[AID]->stop();
            // 连接成功
            cs = 1;
            cm->cfupConnected_(this);
            hbt.start(hbtTime);
        }
    } else if (sendWnd.contains(AID)) sendWnd[AID]->stop();
}

void CFUP::cmdRC_ACK_(bool RT, const QByteArray &data) {
    if (cs == 0 && initiative && data.size() == 13) {
        unsigned short SID = *(unsigned short *) (data.data() + 1);
        long long time = *(long long *) (data.data() + 3);
        unsigned short AID = *(unsigned short *) (data.data() + 11);
        if (SID != 0 || AID != 0) return;
        if (!time_(SID, time))return;
        ID = 1;
        OID = 0;
        delete sendWnd[0];
        sendWnd.remove(0);
        NA_ACK_(0);
        // 连接成功
        cs = 1;
        cm->cfupConnected_(this);
        hbt.start(hbtTime);
    } else if (RT)NA_ACK_(0);
}

void CFUP::cmdC_(bool NA, bool UD, const QByteArray &data) {
    if (!NA) return; // NA必须有
    QByteArray userData;
    if (UD)userData = data.mid(1);
    close(userData);
}

void CFUP::cmdH_(bool RT, const QByteArray &data) {
    if (cs != 1 || data.size() != 11)return;
    unsigned short SID = (*(unsigned short *) (data.data() + 1));
    long long time = *(long long *) (data.data() + 3);
    if (!time_(SID, time))return;
    NA_ACK_(SID);
    if (SID == OID + 1) {
        OID = SID;
        hbt.stop();
        hbt.start(hbtTime);
    } else if (!RT)
        close("心跳包ID不正确");
}
