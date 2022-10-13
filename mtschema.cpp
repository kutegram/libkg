//Generated code.

#include "mtschema.h"

void readMTVector(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 481674261:
        obj["_"] = conId.toInt();
    break;
    }
    i = obj;
}

void writeMTVector(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 481674261:
        writeInt32(stream, obj["_"], callback);
    break;
    }
}

void readMTResPQ(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 85337187:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readByteArray(stream, obj["pq"], callback);
        readVector(stream, obj["server_public_key_fingerprints"], (void*) &readInt64);
    break;
    }
    i = obj;
}

void writeMTResPQ(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 85337187:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeByteArray(stream, obj["pq"], callback);
        writeVector(stream, obj["server_public_key_fingerprints"], (void*) &writeInt64);
    break;
    }
}

void readMTPQInnerData(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1443537003:
        obj["_"] = conId.toInt();
        readByteArray(stream, obj["pq"], callback);
        readByteArray(stream, obj["p"], callback);
        readByteArray(stream, obj["q"], callback);
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt256(stream, obj["new_nonce"], callback);
        readInt32(stream, obj["dc"], callback);
    break;
    case 1459478408:
        obj["_"] = conId.toInt();
        readByteArray(stream, obj["pq"], callback);
        readByteArray(stream, obj["p"], callback);
        readByteArray(stream, obj["q"], callback);
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt256(stream, obj["new_nonce"], callback);
        readInt32(stream, obj["dc"], callback);
        readInt32(stream, obj["expires_in"], callback);
    break;
    }
    i = obj;
}

void writeMTPQInnerData(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1443537003:
        writeInt32(stream, obj["_"], callback);
        writeByteArray(stream, obj["pq"], callback);
        writeByteArray(stream, obj["p"], callback);
        writeByteArray(stream, obj["q"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt256(stream, obj["new_nonce"], callback);
        writeInt32(stream, obj["dc"], callback);
    break;
    case 1459478408:
        writeInt32(stream, obj["_"], callback);
        writeByteArray(stream, obj["pq"], callback);
        writeByteArray(stream, obj["p"], callback);
        writeByteArray(stream, obj["q"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt256(stream, obj["new_nonce"], callback);
        writeInt32(stream, obj["dc"], callback);
        writeInt32(stream, obj["expires_in"], callback);
    break;
    }
}

void readMTServerDHParams(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -790100132:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readByteArray(stream, obj["encrypted_answer"], callback);
    break;
    }
    i = obj;
}

void writeMTServerDHParams(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -790100132:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeByteArray(stream, obj["encrypted_answer"], callback);
    break;
    }
}

void readMTServerDHInnerData(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1249309254:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt32(stream, obj["g"], callback);
        readByteArray(stream, obj["dh_prime"], callback);
        readByteArray(stream, obj["g_a"], callback);
        readInt32(stream, obj["server_time"], callback);
    break;
    }
    i = obj;
}

void writeMTServerDHInnerData(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1249309254:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt32(stream, obj["g"], callback);
        writeByteArray(stream, obj["dh_prime"], callback);
        writeByteArray(stream, obj["g_a"], callback);
        writeInt32(stream, obj["server_time"], callback);
    break;
    }
}

void readMTClientDHInnerData(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1715713620:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt64(stream, obj["retry_id"], callback);
        readByteArray(stream, obj["g_b"], callback);
    break;
    }
    i = obj;
}

void writeMTClientDHInnerData(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1715713620:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt64(stream, obj["retry_id"], callback);
        writeByteArray(stream, obj["g_b"], callback);
    break;
    }
}

void readMTSetClientDHParamsAnswer(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1003222836:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt128(stream, obj["new_nonce_hash1"], callback);
    break;
    case 1188831161:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt128(stream, obj["new_nonce_hash2"], callback);
    break;
    case -1499615742:
        obj["_"] = conId.toInt();
        readInt128(stream, obj["nonce"], callback);
        readInt128(stream, obj["server_nonce"], callback);
        readInt128(stream, obj["new_nonce_hash3"], callback);
    break;
    }
    i = obj;
}

void writeMTSetClientDHParamsAnswer(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1003222836:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt128(stream, obj["new_nonce_hash1"], callback);
    break;
    case 1188831161:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt128(stream, obj["new_nonce_hash2"], callback);
    break;
    case -1499615742:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeInt128(stream, obj["new_nonce_hash3"], callback);
    break;
    }
}

void readMTBindAuthKeyInner(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1973679973:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["nonce"], callback);
        readInt64(stream, obj["temp_auth_key_id"], callback);
        readInt64(stream, obj["perm_auth_key_id"], callback);
        readInt64(stream, obj["temp_session_id"], callback);
        readInt32(stream, obj["expires_at"], callback);
    break;
    }
    i = obj;
}

void writeMTBindAuthKeyInner(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1973679973:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["nonce"], callback);
        writeInt64(stream, obj["temp_auth_key_id"], callback);
        writeInt64(stream, obj["perm_auth_key_id"], callback);
        writeInt64(stream, obj["temp_session_id"], callback);
        writeInt32(stream, obj["expires_at"], callback);
    break;
    }
}

void readMTRpcError(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 558156313:
        obj["_"] = conId.toInt();
        readInt32(stream, obj["error_code"], callback);
        readString(stream, obj["error_message"], callback);
    break;
    }
    i = obj;
}

void writeMTRpcError(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 558156313:
        writeInt32(stream, obj["_"], callback);
        writeInt32(stream, obj["error_code"], callback);
        writeString(stream, obj["error_message"], callback);
    break;
    }
}

void readMTRpcDropAnswer(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1579864942:
        obj["_"] = conId.toInt();
    break;
    case -847714938:
        obj["_"] = conId.toInt();
    break;
    case -1539647305:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["msg_id"], callback);
        readInt32(stream, obj["seq_no"], callback);
        readInt32(stream, obj["bytes"], callback);
    break;
    }
    i = obj;
}

void writeMTRpcDropAnswer(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1579864942:
        writeInt32(stream, obj["_"], callback);
    break;
    case -847714938:
        writeInt32(stream, obj["_"], callback);
    break;
    case -1539647305:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["msg_id"], callback);
        writeInt32(stream, obj["seq_no"], callback);
        writeInt32(stream, obj["bytes"], callback);
    break;
    }
}

void readMTFutureSalt(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 155834844:
        obj["_"] = conId.toInt();
        readInt32(stream, obj["valid_since"], callback);
        readInt32(stream, obj["valid_until"], callback);
        readInt64(stream, obj["salt"], callback);
    break;
    }
    i = obj;
}

void writeMTFutureSalt(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 155834844:
        writeInt32(stream, obj["_"], callback);
        writeInt32(stream, obj["valid_since"], callback);
        writeInt32(stream, obj["valid_until"], callback);
        writeInt64(stream, obj["salt"], callback);
    break;
    }
}

void readMTFutureSalts(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1370486635:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["req_msg_id"], callback);
        readInt32(stream, obj["now"], callback);
        readVector(stream, obj["salts"], (void*) &readMTFutureSalt);
    break;
    }
    i = obj;
}

void writeMTFutureSalts(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1370486635:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["req_msg_id"], callback);
        writeInt32(stream, obj["now"], callback);
        writeVector(stream, obj["salts"], (void*) &writeMTFutureSalt);
    break;
    }
}

void readMTPong(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 880243653:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["msg_id"], callback);
        readInt64(stream, obj["ping_id"], callback);
    break;
    }
    i = obj;
}

void writeMTPong(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 880243653:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["msg_id"], callback);
        writeInt64(stream, obj["ping_id"], callback);
    break;
    }
}

void readMTDestroySessionRes(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -501201412:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["session_id"], callback);
    break;
    case 1658015945:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["session_id"], callback);
    break;
    }
    i = obj;
}

void writeMTDestroySessionRes(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -501201412:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["session_id"], callback);
    break;
    case 1658015945:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["session_id"], callback);
    break;
    }
}

void readMTNewSession(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1631450872:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["first_msg_id"], callback);
        readInt64(stream, obj["unique_id"], callback);
        readInt64(stream, obj["server_salt"], callback);
    break;
    }
    i = obj;
}

void writeMTNewSession(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1631450872:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["first_msg_id"], callback);
        writeInt64(stream, obj["unique_id"], callback);
        writeInt64(stream, obj["server_salt"], callback);
    break;
    }
}

void readMTObject(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 812830625:
        obj["_"] = conId.toInt();
        readByteArray(stream, obj["packed_data"], callback);
    break;
    }
    i = obj;
}

void writeMTObject(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 812830625:
        writeInt32(stream, obj["_"], callback);
        writeByteArray(stream, obj["packed_data"], callback);
    break;
    }
}

void readMTMsgsAck(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1658238041:
        obj["_"] = conId.toInt();
        readVector(stream, obj["msg_ids"], (void*) &readInt64);
    break;
    }
    i = obj;
}

void writeMTMsgsAck(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1658238041:
        writeInt32(stream, obj["_"], callback);
        writeVector(stream, obj["msg_ids"], (void*) &writeInt64);
    break;
    }
}

void readMTBadMsgNotification(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1477445615:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["bad_msg_id"], callback);
        readInt32(stream, obj["bad_msg_seqno"], callback);
        readInt32(stream, obj["error_code"], callback);
    break;
    case -307542917:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["bad_msg_id"], callback);
        readInt32(stream, obj["bad_msg_seqno"], callback);
        readInt32(stream, obj["error_code"], callback);
        readInt64(stream, obj["new_server_salt"], callback);
    break;
    }
    i = obj;
}

void writeMTBadMsgNotification(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1477445615:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["bad_msg_id"], callback);
        writeInt32(stream, obj["bad_msg_seqno"], callback);
        writeInt32(stream, obj["error_code"], callback);
    break;
    case -307542917:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["bad_msg_id"], callback);
        writeInt32(stream, obj["bad_msg_seqno"], callback);
        writeInt32(stream, obj["error_code"], callback);
        writeInt64(stream, obj["new_server_salt"], callback);
    break;
    }
}

void readMTMsgResendReq(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 2105940488:
        obj["_"] = conId.toInt();
        readVector(stream, obj["msg_ids"], (void*) &readInt64);
    break;
    }
    i = obj;
}

void writeMTMsgResendReq(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 2105940488:
        writeInt32(stream, obj["_"], callback);
        writeVector(stream, obj["msg_ids"], (void*) &writeInt64);
    break;
    }
}

void readMTMsgsStateReq(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -630588590:
        obj["_"] = conId.toInt();
        readVector(stream, obj["msg_ids"], (void*) &readInt64);
    break;
    }
    i = obj;
}

void writeMTMsgsStateReq(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -630588590:
        writeInt32(stream, obj["_"], callback);
        writeVector(stream, obj["msg_ids"], (void*) &writeInt64);
    break;
    }
}

void readMTMsgsStateInfo(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 81704317:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["req_msg_id"], callback);
        readByteArray(stream, obj["info"], callback);
    break;
    }
    i = obj;
}

void writeMTMsgsStateInfo(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 81704317:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["req_msg_id"], callback);
        writeByteArray(stream, obj["info"], callback);
    break;
    }
}

void readMTMsgsAllInfo(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1933520591:
        obj["_"] = conId.toInt();
        readVector(stream, obj["msg_ids"], (void*) &readInt64);
        readByteArray(stream, obj["info"], callback);
    break;
    }
    i = obj;
}

void writeMTMsgsAllInfo(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1933520591:
        writeInt32(stream, obj["_"], callback);
        writeVector(stream, obj["msg_ids"], (void*) &writeInt64);
        writeByteArray(stream, obj["info"], callback);
    break;
    }
}

void readMTMsgDetailedInfo(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 661470918:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["msg_id"], callback);
        readInt64(stream, obj["answer_msg_id"], callback);
        readInt32(stream, obj["bytes"], callback);
        readInt32(stream, obj["status"], callback);
    break;
    case -2137147681:
        obj["_"] = conId.toInt();
        readInt64(stream, obj["answer_msg_id"], callback);
        readInt32(stream, obj["bytes"], callback);
        readInt32(stream, obj["status"], callback);
    break;
    }
    i = obj;
}

void writeMTMsgDetailedInfo(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 661470918:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["msg_id"], callback);
        writeInt64(stream, obj["answer_msg_id"], callback);
        writeInt32(stream, obj["bytes"], callback);
        writeInt32(stream, obj["status"], callback);
    break;
    case -2137147681:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["answer_msg_id"], callback);
        writeInt32(stream, obj["bytes"], callback);
        writeInt32(stream, obj["status"], callback);
    break;
    }
}

void readMTDestroyAuthKeyRes(TelegramStream &stream, QVariant &i, void* callback)
{
    TelegramObject obj;
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -161422892:
        obj["_"] = conId.toInt();
    break;
    case 178201177:
        obj["_"] = conId.toInt();
    break;
    case -368010477:
        obj["_"] = conId.toInt();
    break;
    }
    i = obj;
}

void writeMTDestroyAuthKeyRes(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -161422892:
        writeInt32(stream, obj["_"], callback);
    break;
    case 178201177:
        writeInt32(stream, obj["_"], callback);
    break;
    case -368010477:
        writeInt32(stream, obj["_"], callback);
    break;
    }
}

void readMTMethodReqPqMulti(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1099002127:
    readMTResPQ(stream, i, callback);
    break;
    }
}

void writeMTMethodReqPqMulti(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1099002127:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
    break;
    }
}

void readMTMethodReqDHParams(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -686627650:
    readMTServerDHParams(stream, i, callback);
    break;
    }
}

void writeMTMethodReqDHParams(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -686627650:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeByteArray(stream, obj["p"], callback);
        writeByteArray(stream, obj["q"], callback);
        writeInt64(stream, obj["public_key_fingerprint"], callback);
        writeByteArray(stream, obj["encrypted_data"], callback);
    break;
    }
}

void readMTMethodSetClientDHParams(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -184262881:
    readMTSetClientDHParamsAnswer(stream, i, callback);
    break;
    }
}

void writeMTMethodSetClientDHParams(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -184262881:
        writeInt32(stream, obj["_"], callback);
        writeInt128(stream, obj["nonce"], callback);
        writeInt128(stream, obj["server_nonce"], callback);
        writeByteArray(stream, obj["encrypted_data"], callback);
    break;
    }
}

void readMTMethodRpcDropAnswer(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 1491380032:
    readMTRpcDropAnswer(stream, i, callback);
    break;
    }
}

void writeMTMethodRpcDropAnswer(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 1491380032:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["req_msg_id"], callback);
    break;
    }
}

void readMTMethodGetFutureSalts(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1188971260:
    readMTFutureSalts(stream, i, callback);
    break;
    }
}

void writeMTMethodGetFutureSalts(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1188971260:
        writeInt32(stream, obj["_"], callback);
        writeInt32(stream, obj["num"], callback);
    break;
    }
}

void readMTMethodPing(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case 2059302892:
    readMTPong(stream, i, callback);
    break;
    }
}

void writeMTMethodPing(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case 2059302892:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["ping_id"], callback);
    break;
    }
}

void readMTMethodPingDelayDisconnect(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -213746804:
    readMTPong(stream, i, callback);
    break;
    }
}

void writeMTMethodPingDelayDisconnect(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -213746804:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["ping_id"], callback);
        writeInt32(stream, obj["disconnect_delay"], callback);
    break;
    }
}

void readMTMethodDestroySession(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -414113498:
    readMTDestroySessionRes(stream, i, callback);
    break;
    }
}

void writeMTMethodDestroySession(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -414113498:
        writeInt32(stream, obj["_"], callback);
        writeInt64(stream, obj["session_id"], callback);
    break;
    }
}

void readMTMethodHttpWait(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -1835453025:
        break;
    }
}

void writeMTMethodHttpWait(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -1835453025:
        writeInt32(stream, obj["_"], callback);
        writeInt32(stream, obj["max_delay"], callback);
        writeInt32(stream, obj["wait_after"], callback);
        writeInt32(stream, obj["max_wait"], callback);
    break;
    }
}

void readMTMethodDestroyAuthKey(TelegramStream &stream, QVariant &i, void* callback)
{
    QVariant conId;
    readInt32(stream, conId, callback);
    switch (conId.toInt()) {
    case -784117408:
    readMTDestroyAuthKeyRes(stream, i, callback);
    break;
    }
}

void writeMTMethodDestroyAuthKey(TelegramStream &stream, QVariant i, void* callback)
{
    TelegramObject obj = i.toMap();
    switch (obj["_"].toInt()) {
    case -784117408:
        writeInt32(stream, obj["_"], callback);
    break;
    }
}

