#include "tgtransport.h"

#include "tgclient.h"
#include <QDateTime>
#include "mtschema.h"
#include "crypto.h"
#include <QtEndian>
#include <QList>
#include <cstdlib>

TgTransport::TgTransport(TgClient *parent)
    : QObject(parent)
    , _client(parent)
    , _socket(parent->socket())
    , nonce()
    , serverNonce()
    , newNonce()
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
    writeInt64(packet, QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
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

    processMessage();
}

void TgTransport::readIntermediate(QByteArray &data)
{
    QByteArray buffer = _socket->read(4);
    if (buffer.length() < 4)
        buffer.append(QByteArray(4 - buffer.length(), 0));

    TgPacket packet(buffer);
    QVariant length;
    readInt32(packet, length);

    data = _socket->read(length.toInt());
}

void TgTransport::processMessage()
{
    QByteArray message;
    readIntermediate(message);

    TgPacket packet(message);

    QVariant id;
    readInt64(packet, id);

    if (message.length() <= 4) {
        qDebug() << "Got MTProto error:" << id.toInt();
        return;
    }

    if (id != 0) {
        qDebug() << "MTProto messages are not supported yet";
        return;
    }

    QVariant messageId;
    readInt64(packet, messageId);

    QVariant dataLength;
    readInt32(packet, dataLength);

    QByteArray data = message.mid(20, dataLength.toInt());

    //qDebug() << "[IN ]" << data.toHex();

    QVariant conId;
    readInt32(packet, conId);

    handleObject(conId.toInt(), data);
}

void TgTransport::handleObject(qint32 conId, QByteArray data)
{
    qDebug() << "Got an object:" << conId;

    switch (conId) {
    case MTType::ResPQ:
        handleResPQ(data);
        break;
    case MTType::ServerDHParamsOk:
        handleServerDHParamsOk(data);
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
    //quint32 q = pq / p;
    //Symbian compilation workaround
    lldiv_t output;
    output = lldiv(pq, p);
    quint32 q = output.quot;
    if (p > q) qSwap(p, q);

    QByteArray pBytes(INT32_BYTES, 0);
    qToBigEndian(p, (uchar*) pBytes.data());

    QByteArray qBytes(INT32_BYTES, 0);
    qToBigEndian(q, (uchar*) qBytes.data());

    QList<DHKey> keychain = QList<DHKey>() << DHKey("00e8bb3305c0b52c6cf2afdf763731"
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
                                                    "cff3", -3414540481677951611LL, "10001")
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
                                                    "e7f5", -5595554452916591101LL, "10001");
    TgVector serverKeyFingerprints = obj["server_public_key_fingerprints"].toList();

    QList<DHKey> matched;

    for (qint32 i = 0; i < serverKeyFingerprints.length(); ++i) {
        for (qint32 j = 0; j < keychain.length(); ++j) {
            if (serverKeyFingerprints[i].toLongLong() == keychain[j].fingerprint) {
                matched << keychain[j];
                break;
            }
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
    //If DC is production - just DC id
    //If DC is testing - DC id + 10000
    //If DC is Media Only - DC id * -1

    TgPacket pidPacket;
    writeMTPQInnerData(pidPacket, pqInnerData);
    QByteArray pidData = pidPacket.toByteArray();

    if (pidData.length() > 144) {
        qDebug() << "SECURITY ERROR: pqInnerData is longer that 144 bytes";
        return;
    }

    QByteArray encryptedData = rsaPad(pidData, matched.first());

    qDebug() << encryptedData.size();

    TGOBJECT(MTType::ReqDHParamsMethod, reqDH);
    reqDH["nonce"] = nonce;
    reqDH["server_nonce"] = serverNonce;
    reqDH["p"] = pBytes;
    reqDH["q"] = qBytes;
    reqDH["public_key_fingerprint"] = matched.first().fingerprint;
    reqDH["encrypted_data"] = encryptedData;

    sendPlainObject<&writeMTMethodReqDHParams>(reqDH);

    qDebug() << "DH exchange: step 4";
}

void TgTransport::handleServerDHParamsOk(QByteArray data)
{
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
}

void TgTransport::_bytesSent(qint64 count)
{
    qDebug() << "Sent" << count << "bytes";
}
