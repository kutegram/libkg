#ifndef MTSCHEMA_H
#define MTSCHEMA_H

//Generated code.

#include "tgstream.h"

namespace MTType {
enum Types
{
    Unknown = 0,
    Vector = 481674261, //0x1cb5c415
    ResPQ = 85337187, //0x05162463
    PQInnerDataDc = -1443537003, //0xa9f55f95
    PQInnerDataTempDc = 1459478408, //0x56fddf88
    ServerDHParamsOk = -790100132, //0xd0e8075c
    ServerDHInnerData = -1249309254, //0xb5890dba
    ClientDHInnerData = 1715713620, //0x6643b654
    DhGenOk = 1003222836, //0x3bcbf734
    DhGenRetry = 1188831161, //0x46dc1fb9
    DhGenFail = -1499615742, //0xa69dae02
    BindAuthKeyInner = 1973679973, //0x75a3f765
    RpcResult = -212046591, //0xf35c6d01
    RpcError = 558156313, //0x2144ca19
    RpcAnswerUnknown = 1579864942, //0x5e2ad36e
    RpcAnswerDroppedRunning = -847714938, //0xcd78e586
    RpcAnswerDropped = -1539647305, //0xa43ad8b7
    FutureSalt = 155834844, //0x0949d9dc
    FutureSalts = -1370486635, //0xae500895
    Pong = 880243653, //0x347773c5
    DestroySessionOk = -501201412, //0xe22045fc
    DestroySessionNone = 1658015945, //0x62d350c9
    NewSessionCreated = -1631450872, //0x9ec20908
    MsgContainer = 1945237724, //0x73f1f8dc
    Message = 1538843921, //0x5bb8e511
    MsgCopy = -530561358, //0xe06046b2
    GzipPacked = 812830625, //0x3072cfa1
    MsgsAck = 1658238041, //0x62d6b459
    BadMsgNotification = -1477445615, //0xa7eff811
    BadServerSalt = -307542917, //0xedab447b
    MsgResendReq = 2105940488, //0x7d861a08
    MsgsStateReq = -630588590, //0xda69fb52
    MsgsStateInfo = 81704317, //0x04deb57d
    MsgsAllInfo = -1933520591, //0x8cc0d131
    MsgDetailedInfo = 661470918, //0x276d3ec6
    MsgNewDetailedInfo = -2137147681, //0x809db6df
    DestroyAuthKeyOk = -161422892, //0xf660e1d4
    DestroyAuthKeyNone = 178201177, //0x0a9f2259
    DestroyAuthKeyFail = -368010477, //0xea109b13
    ReqPqMultiMethod = -1099002127, //0xbe7e8ef1
    ReqDHParamsMethod = -686627650, //0xd712e4be
    SetClientDHParamsMethod = -184262881, //0xf5045f1f
    RpcDropAnswerMethod = 1491380032, //0x58e4a740
    GetFutureSaltsMethod = -1188971260, //0xb921bd04
    PingMethod = 2059302892, //0x7abe77ec
    PingDelayDisconnectMethod = -213746804, //0xf3427b8c
    DestroySessionMethod = -414113498, //0xe7512126
    HttpWaitMethod = -1835453025, //0x9299359f
    DestroyAuthKeyMethod = -784117408, //0xd1435160
};
}

template <READ_METHOD R, WRITE_METHOD W> void readMTRpcResult(TelegramStream &stream, QVariant &i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void writeMTRpcResult(TelegramStream &stream, QVariant i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void readMTMessageContainer(TelegramStream &stream, QVariant &i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void writeMTMessageContainer(TelegramStream &stream, QVariant i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void readMTMessage(TelegramStream &stream, QVariant &i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void writeMTMessage(TelegramStream &stream, QVariant i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void readMTMessageCopy(TelegramStream &stream, QVariant &i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void writeMTMessageCopy(TelegramStream &stream, QVariant i, void* callback = 0);


void readMTVector(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTVector(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTResPQ(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTResPQ(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTPQInnerData(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTPQInnerData(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTServerDHParams(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTServerDHParams(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTServerDHInnerData(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTServerDHInnerData(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTClientDHInnerData(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTClientDHInnerData(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTSetClientDHParamsAnswer(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTSetClientDHParamsAnswer(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTBindAuthKeyInner(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTBindAuthKeyInner(TelegramStream &stream, QVariant i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void readMTRpcResult(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -212046591:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["req_msg_id"], callback);
        (*R)(stream, obj["result"], callback);
    break;
    }
    i = obj;
}

template <READ_METHOD R, WRITE_METHOD W> void writeMTRpcResult(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -212046591:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["req_msg_id"], callback);
        (*W)(stream, obj["result"], callback);
    break;
    }
}

void readMTRpcError(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTRpcError(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTRpcDropAnswer(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTRpcDropAnswer(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTFutureSalt(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTFutureSalt(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTFutureSalts(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTFutureSalts(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTPong(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTPong(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTDestroySessionRes(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTDestroySessionRes(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTNewSession(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTNewSession(TelegramStream &stream, QVariant i, void* callback = 0);
template <READ_METHOD R, WRITE_METHOD W> void readMTMessageContainer(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1945237724:
        obj["_"] = conId.toInt();
        readVector(stream, obj["messages"], (void*) &readMTMessage<R, W>);
    break;
    }
    i = obj;
}

template <READ_METHOD R, WRITE_METHOD W> void writeMTMessageContainer(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1945237724:
        writeInt32(stream, obj["_"], callback);
        writeVector(stream, obj["messages"], (void*) &writeMTMessage<R, W>);
    break;
    }
}

template <READ_METHOD R, WRITE_METHOD W> void readMTMessage(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    switch (conId.toInt()) {
    case 1538843921:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["msg_id"], callback);
        readInt32(stream, obj["seqno"], callback);
        readInt32(stream, obj["bytes"], callback);
        (*R)(stream, obj["body"], callback);
    break;
    }
    i = obj;
}

template <READ_METHOD R, WRITE_METHOD W> void writeMTMessage(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1538843921:
            writeInt64(stream, obj["msg_id"], callback);
        writeInt32(stream, obj["seqno"], callback);
        writeInt32(stream, obj["bytes"], callback);
        (*W)(stream, obj["body"], callback);
    break;
    }
}

template <READ_METHOD R, WRITE_METHOD W> void readMTMessageCopy(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -530561358:
        obj["_"] = conId.toInt();
        readMTMessage<R, W>(stream, obj["orig_message"], callback);
    break;
    }
    i = obj;
}

template <READ_METHOD R, WRITE_METHOD W> void writeMTMessageCopy(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -530561358:
        writeInt32(stream, obj["_"], callback);
        writeMTMessage<R, W>(stream, obj["orig_message"], callback);
    break;
    }
}

void readMTObject(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTObject(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMsgsAck(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMsgsAck(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTBadMsgNotification(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTBadMsgNotification(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMsgResendReq(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMsgResendReq(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMsgsStateReq(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMsgsStateReq(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMsgsStateInfo(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMsgsStateInfo(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMsgsAllInfo(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMsgsAllInfo(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMsgDetailedInfo(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMsgDetailedInfo(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTDestroyAuthKeyRes(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTDestroyAuthKeyRes(TelegramStream &stream, QVariant i, void* callback = 0);

void readMTMethodReqPqMulti(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodReqPqMulti(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodReqDHParams(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodReqDHParams(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodSetClientDHParams(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodSetClientDHParams(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodRpcDropAnswer(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodRpcDropAnswer(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodGetFutureSalts(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodGetFutureSalts(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodPing(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodPing(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodPingDelayDisconnect(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodPingDelayDisconnect(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodDestroySession(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodDestroySession(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodHttpWait(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodHttpWait(TelegramStream &stream, QVariant i, void* callback = 0);
void readMTMethodDestroyAuthKey(TelegramStream &stream, QVariant &i, void* callback = 0);
void writeMTMethodDestroyAuthKey(TelegramStream &stream, QVariant i, void* callback = 0);

#endif //MTSCHEMA_H

