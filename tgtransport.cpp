#include "tgtransport.h"

#include "tgclient.h"
#include <QDateTime>
#include "mtschema.h"
#include "crypto.h"
#include <QtEndian>
#include <QList>
#include <cstdlib>
#include "tlschema.h"
#include "apisecrets.h"
#include "systemname.h"
#include <QCoreApplication>
#include <QLocale>
#include <QStringList>
#include <qcompressor.h>

TgTransport::TgTransport(TgClient *parent)
    : QObject(parent)
    , _client(parent)
    , _socket(parent->socket())
    , nonce()
    , serverNonce()
    , newNonce()
    , authKey()
    , serverSalt(0)
    , authKeyId(0)
    , timeOffset(0)
    , sequence(0)
    , lastMessageId(0)
    , sessionId(0)
{
    connect(_socket, SIGNAL(readyRead()), this, SLOT(_readyRead()));
    connect(_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(_bytesSent(qint64)));

    TgPacket packet;
    writeInt32(packet, 0xeeeeeeee);

    _socket->write(packet.toByteArray());
}

void TgTransport::sendPlainMessage(QByteArray data)
{
    TgPacket packet;
    writeInt64(packet, 0);
    writeInt64(packet, getNewMessageId());
    writeInt32(packet, data.length());

    sendIntermediate(packet.toByteArray() + data);
}

void TgTransport::sendIntermediate(QByteArray data)
{
    TgPacket packet;
    writeInt32(packet, data.length());

    _socket->write(packet.toByteArray() + data);
}

void TgTransport::authorize()
{
    qDebug() << "DH exchange: step 1";

    TGOBJECT(MTType::ReqPqMultiMethod, reqPq);
    reqPq["nonce"] = nonce = randomBytes(INT128_BYTES);
    sendPlainObject<&writeMTMethodReqPqMulti>(reqPq);
}

void TgTransport::_readyRead()
{
    qDebug() << "Ready to read";

    processMessage(readIntermediate());
}

QByteArray TgTransport::readIntermediate()
{
    QByteArray buffer = _socket->read(4);
    if (buffer.length() < 4)
        buffer.append(QByteArray(4 - buffer.length(), 0));

    TgPacket packet(buffer);
    QVariant length;
    readInt32(packet, length);

    return _socket->read(length.toInt());
}

void TgTransport::processMessage(QByteArray message)
{
    TgPacket packet(message);

    if (message.length() <= 4) {
        QVariant error;
        readInt32(packet, error);
        qDebug() << "Got MTProto error:" << error.toInt();
        return;
    }

    QVariant var;
    readInt64(packet, var);

    QByteArray data;

    if (var.toLongLong() == 0) {
        readInt64(packet, var);
        readInt32(packet, var);
        readRawBytes(packet, data, var.toInt());
    } else {
        //TODO: Important checks
        //TODO: check auth key id
        //TODO: check msg_key correctness
        //TODO: NB: after decryption, msg_key must be equal to SHA-256 of data thus obtained.

        QByteArray messageKey;
        readRawBytes(packet, messageKey, 16);
        QByteArray keyIv;
        QByteArray keyData = calcEncryptionKey(authKey, messageKey, keyIv, false);

        TgPacket plainMessage(decryptAES256IGE(message.mid(24), keyIv, keyData));
        readUInt64(plainMessage, var); //remoteSalt
        readUInt64(plainMessage, var); //remoteId
        readInt64(plainMessage, var); //remoteMessageId
        readInt32(plainMessage, var); //remoteSequence
        readInt32(plainMessage, var); //messageLength
        readRawBytes(plainMessage, data, var.toInt());
    }

    //qDebug() << "[IN ]" << data.toHex();

    handleObject(data);
}

void TgTransport::handleObject(QByteArray data)
{
    TgPacket packet(data);
    QVariant var;
    readInt32(packet, var);
    qint32 conId = var.toInt();

    qDebug() << "Got an object:" << conId;

    switch (conId) {
    case MTType::ResPQ:
        handleResPQ(data);
        break;
    case MTType::ServerDHParamsOk:
        handleServerDHParamsOk(data);
        break;
    case MTType::DhGenOk:
        handleDhGenOk(data);
        break;
    default:
        qDebug() << "WARNING: object" << conId << "is unknown!";
        break;
    }
}

void TgTransport::handleResPQ(QByteArray data)
{
    TgPacket packet(data);
    QVariant var;

    readMTResPQ(packet, var);
    TgObject obj = var.toMap();

    qDebug() << "DH exchange: step 2";

    if (obj["nonce"].toByteArray() != nonce) {
        qDebug() << "SECURITY ERROR: nonces are different";
        return;
    }

    serverNonce = obj["server_nonce"].toByteArray();

    qDebug() << "DH exchange: step 3";

    QByteArray pqBytes = obj["pq"].toByteArray();
    quint64 pq = qFromBigEndian<quint64>((const uchar*) pqBytes.constData());
    quint32 p = findDivider(pq);
    quint32 q = pq / p;
    if (p > q) qSwap(p, q);

    QByteArray pBytes(INT32_BYTES, 0);
    qToBigEndian(p, (uchar*) pBytes.data());

    QByteArray qBytes(INT32_BYTES, 0);
    qToBigEndian(q, (uchar*) qBytes.data());

    QList<DHKey> keychain = QList<DHKey>()
                                           << DHKey("00e8bb3305c0b52c6cf2afdf763731"
                                                    "3489e63e05268e5badb601af417786"
                                                    "472e5f93b85438968e20e6729a301c"
                                                    "0afc121bf7151f834436f7fda68084"
                                                    "7a66bf64accec78ee21c0b316f0eda"
                                                    "fe2f41908da7bd1f4a5107638eeb67"
                                                    "040ace472a14f90d9f7c2b7def9968"
                                                    "8ba3073adb5750bb02964902a359fe"
                                                    "745d8170e36876d4fd8a5d41b2a76c"
                                                    "bff9a13267eb9580b2d06d10357448"
                                                    "d20d9da2191cb5d8c93982961cdfde"
                                                    "da629e37f1fb09a0722027696032fe"
                                                    "61ed663db7a37f6f263d370f69db53"
                                                    "a0dc0a1748bdaaff6209d5645485e6"
                                                    "e001d1953255757e4b8e42813347b1"
                                                    "1da6ab500fd0ace7e6dfa3736199cc"
                                                    "af9397ed0745a427dcfa6cd67bcb1a"
                                                    "cff3", -3414540481677951611LL)
                                           << DHKey("00c8c11d635691fac091dd9489aedc"
                                                    "ed2932aa8a0bcefef05fa800892d9b"
                                                    "52ed03200865c9e97211cb2ee6c7ae"
                                                    "96d3fb0e15aeffd66019b44a08a240"
                                                    "cfdd2868a85e1f54d6fa5deaa041f6"
                                                    "941ddf302690d61dc476385c2fa655"
                                                    "142353cb4e4b59f6e5b6584db76fe8"
                                                    "b1370263246c010c93d011014113eb"
                                                    "df987d093f9d37c2be48352d69a168"
                                                    "3f8f6e6c2167983c761e3ab169fde5"
                                                    "daaa12123fa1beab621e4da5935e9c"
                                                    "198f82f35eae583a99386d8110ea6b"
                                                    "d1abb0f568759f62694419ea5f6984"
                                                    "7c43462abef858b4cb5edc84e7b922"
                                                    "6cd7bd7e183aa974a712c079dde85b"
                                                    "9dc063b8a5c08e8f859c0ee5dcd824"
                                                    "c7807f20153361a7f63cfd2a433a1b"
                                                    "e7f5", -5595554452916591101LL);

    TgVector serverKeyFingerprints = obj["server_public_key_fingerprints"].toList();

    QList<DHKey> matched;

    for (qint32 i = 0; i < serverKeyFingerprints.length(); ++i) {
        for (qint32 j = 0; j < keychain.length(); ++j) {
            if (serverKeyFingerprints[i].toLongLong() == keychain[j].fingerprint) {
                matched << keychain[j];
                qDebug() << "Matched fingerprint: " << keychain[j].fingerprint;
                break;
            }
        }
        if (!matched.isEmpty()) {
            break;
        }
    }

    if (matched.isEmpty()) {
        qDebug() << "SECURITY ERROR: no suitable keys are found";
        return;
    }

    TGOBJECT(MTType::PQInnerDataDc, pqInnerData);
    pqInnerData["pq"] = pqBytes;
    pqInnerData["p"] = pBytes;
    pqInnerData["q"] = qBytes;
    pqInnerData["nonce"] = nonce;
    pqInnerData["server_nonce"] = serverNonce;
    pqInnerData["new_nonce"] = newNonce = randomBytes(INT256_BYTES);
    pqInnerData["dc"] = 10002; //TODO: remove hardcoded DC ID
    //If DC is testing - DC id + 10000
    //If DC is Media Only - DC id * -1

    TgPacket pidPacket;
    writeMTPQInnerData(pidPacket, pqInnerData);
    QByteArray pidData = pidPacket.toByteArray();

    if (pidData.length() > 144) {
        qDebug() << "SECURITY ERROR: pqInnerData is longer that 144 bytes";
        return;
    }

    QByteArray encryptedData = rsaPad(pidData, matched[0]);
    encryptedData.insert(0, QByteArray(256 - encryptedData.size(), 0));

    TGOBJECT(MTType::ReqDHParamsMethod, reqDH);
    reqDH["nonce"] = nonce;
    reqDH["server_nonce"] = serverNonce;
    reqDH["p"] = pBytes;
    reqDH["q"] = qBytes;
    reqDH["public_key_fingerprint"] = matched[0].fingerprint;
    reqDH["encrypted_data"] = encryptedData;

    sendPlainObject<&writeMTMethodReqDHParams>(reqDH);

    qDebug() << "DH exchange: step 4";
}

void TgTransport::handleServerDHParamsOk(QByteArray data)
{
    qint32 localTime = QDateTime::currentDateTimeUtc().toTime_t();

    TgPacket packet(data);
    QVariant var;

    readMTServerDHParams(packet, var);
    TgObject obj = var.toMap();

    qDebug() << "DH exchange: step 5";

    if (obj["nonce"].toByteArray() != nonce) {
        qDebug() << "SECURITY ERROR: nonces are different";
        return;
    }

    if (obj["server_nonce"].toByteArray() != serverNonce) {
        qDebug() << "SECURITY ERROR: server nonces are different";
        return;
    }

    QByteArray encryptedAnswer = obj["encrypted_answer"].toByteArray();
    QByteArray tempAesKey = hashSHA1(newNonce + serverNonce) + hashSHA1(serverNonce + newNonce).mid(0, 12);
    QByteArray tempAesIv = hashSHA1(serverNonce + newNonce).mid(12, 8) + hashSHA1(newNonce + newNonce) + newNonce.mid(0, 4);

    TgPacket answerPacket(decryptAES256IGE(encryptedAnswer, tempAesIv, tempAesKey));
    QByteArray answerHash;
    readRawBytes(answerPacket, answerHash, 20);
    //TODO: verify SHA1
    readMTServerDHInnerData(answerPacket, var);

    TgObject innerData = var.toMap();

    QByteArray bBytes = randomBytes(256);
    qint32 gValue = innerData["g"].toInt();
    QByteArray gBytes(4, 0);
    qToBigEndian(gValue, (uchar*) gBytes.data());
    QByteArray dhPrime = innerData["dh_prime"].toByteArray();
    //TODO: check dh prime (cpu expensive!)

    QByteArray gbBytes = encryptRSA(gBytes, dhPrime, bBytes);

    TGOBJECT(MTType::ClientDHInnerData, clientInnerData);
    clientInnerData["nonce"] = nonce;
    clientInnerData["server_nonce"] = serverNonce;
    clientInnerData["retry_id"] = 0;
    //TODO: retry ID support
    clientInnerData["g_b"] = gbBytes;

    TgPacket clientInnerDataPacket;
    writeMTClientDHInnerData(clientInnerDataPacket, clientInnerData);
    QByteArray cidBytes = clientInnerDataPacket.toByteArray();
    QByteArray dataWithHash = hashSHA1(cidBytes) + cidBytes;
    dataWithHash += randomBytes(16 - dataWithHash.size() % 16);

    QByteArray encryptedData = encryptAES256IGE(dataWithHash, tempAesIv, tempAesKey);

    TGOBJECT(MTType::SetClientDHParamsMethod, setParams);
    setParams["nonce"] = nonce;
    setParams["server_nonce"] = serverNonce;
    setParams["encrypted_data"] = encryptedData;

    sessionId = qFromLittleEndian<quint64>((const uchar*) randomBytes(INT64_BYTES).constData());
    QByteArray gaBytes = innerData["g_a"].toByteArray();
    authKey = encryptRSA(gaBytes, dhPrime, bBytes);
    authKey.insert(0, QByteArray(256 - authKey.size(), 0));
    serverSalt = qFromLittleEndian<quint64>((const uchar*) xorArray(newNonce.mid(0, 8), serverNonce.mid(0, 8)).constData());
    authKeyId = qFromLittleEndian<quint64>((const uchar*) hashSHA1(authKey).mid(12, 8).constData());
    timeOffset = innerData["server_time"].toInt() - localTime;
    sequence = 0;
    lastMessageId = 0;

    sendPlainObject<&writeMTMethodSetClientDHParams>(setParams);
}

void TgTransport::handleDhGenOk(QByteArray data)
{
    TgPacket packet(data);
    QVariant var;

    readMTSetClientDHParamsAnswer(packet, var);
    TgObject obj = var.toMap();

    if (obj["nonce"].toByteArray() != nonce) {
        qDebug() << "SECURITY ERROR: nonces are different";
        return;
    }

    if (obj["server_nonce"].toByteArray() != serverNonce) {
        qDebug() << "SECURITY ERROR: server nonces are different";
        return;
    }

    QByteArray newNonceHash1 = hashSHA1(newNonce + QByteArray(1, 1) + hashSHA1(authKey).mid(0, 8)).mid(4);
    if (obj["new_nonce_hash1"].toByteArray() != newNonceHash1) {
        qDebug() << "SECURITY ERROR: new nonce hashes are different";
        return;
    }

    //TODO: important checks
    qDebug() << "DH completed";

    initConnection();
}

void TgTransport::initConnection()
{
    TGOBJECT(TLType::HelpGetConfigMethod, getConfig);
    TGOBJECT(TLType::InitConnectionMethod, initConnection);

    char id[6] = {API_ID_BYTES};
    initConnection["api_id"] = QString::fromAscii(id, 6).toInt();
    initConnection["device_model"] = systemName() + "-based device";
    initConnection["system_version"] = systemName();
    QString appVersion = QCoreApplication::applicationVersion();
    if (appVersion.isEmpty())
        appVersion = "1.0.0";
    initConnection["app_version"] = appVersion;
    initConnection["system_lang_code"] = QLocale::system().name().split("_")[0];
    initConnection["lang_pack"] = ""; //TODO: what is lang pack?
    initConnection["lang_code"] = QLocale::system().name().split("_")[0];
    initConnection["query"] = getConfig;

    TGOBJECT(TLType::InvokeWithLayerMethod, invokeWithLayer);
    invokeWithLayer["layer"] = API_LAYER;
    invokeWithLayer["query"] = initConnection;

    sendMTObject< &writeTLMethodInvokeWithLayer
                < &readTLMethodInitConnection
                < &readTLMethodHelpGetConfig ,
                &writeTLMethodHelpGetConfig > ,
                &writeTLMethodInitConnection
                < &readTLMethodHelpGetConfig ,
                &writeTLMethodHelpGetConfig > > >(invokeWithLayer);
}

QByteArray TgTransport::gzipPacket(QByteArray data)
{
    QByteArray packedData;
    if (!QCompressor::gzipCompress(data, packedData)) {
        qDebug() << "[ERROR] Gzip compression error.";
        return data;
    }

    TGOBJECT(MTType::GzipPacked, zipped);
    zipped["packed_data"] = packedData;

    TgPacket packet;
    writeMTObject(packet, zipped);
    return packet.toByteArray();
}

void TgTransport::sendMTMessage(QByteArray data)
{
    if (data.size() > 255)
        data = gzipPacket(data);

    TgPacket packet;
    writeUInt64(packet, serverSalt);
    writeUInt64(packet, sessionId);
    writeInt64(packet, getNewMessageId());
    writeInt32(packet, generateSequence(true));
    writeInt32(packet, data.length());
    writeRawBytes(packet, data);
    writeRawBytes(packet, randomBytes((0x7FFFFFF0 - data.length()) % 16 + (randomInt(14) + 2) * 16));

    QByteArray packetBytes = packet.toByteArray();
    QByteArray messageKey = calcMessageKey(authKey, packetBytes);
    QByteArray keyIv;
    QByteArray keyData = calcEncryptionKey(authKey, messageKey, keyIv, true);
    QByteArray cipherText = encryptAES256IGE(packetBytes, keyIv, keyData);

    TgPacket cipherPacket;
    writeUInt64(cipherPacket, authKeyId);
    writeRawBytes(cipherPacket, messageKey);
    writeRawBytes(cipherPacket, cipherText);

    sendIntermediate(cipherPacket.toByteArray());
}

qint64 TgTransport::getNewMessageId()
{
    qint64 time = QDateTime::currentDateTime().toUTC().toTime_t();
    qint64 newMessageId = ((time + timeOffset) << 32) | (((time + timeOffset) % 1000) << 22);
    if (lastMessageId >= newMessageId) newMessageId = lastMessageId + 4;
    lastMessageId = newMessageId;
    return newMessageId;
}

qint32 TgTransport::generateSequence(bool isContent)
{
    qint32 result = isContent ? sequence++ * 2 + 1 : sequence * 2;
    return result;
}

void TgTransport::_bytesSent(qint64 count)
{
    qDebug() << "Sent" << count << "bytes";
}
