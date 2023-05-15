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
#include <QSettings>

TgTransport::TgTransport(TgClient *parent, QString sessionName, qint32 dcId)
    : QObject(parent)
    , _client(parent)
    , _socket(0)
    , _timer()

    , testMode(false) //Change it for debugging or whatever
    , mediaOnly(false) //Change it for debugging or whatever

    , currentDc(dcId)
    , currentPort(0)
    , currentHost()
    , isMain(dcId == 0)

    , tgConfig()

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
    , pingId(0)
    , userId(0)

    , pendingMessages()
    , migrationMessages()

    , _sessionName(sessionName)

    , msgsToAck()

    , authCheckMsgId(0)

    , initialized(false)
{
    if (_sessionName.isEmpty()) {
        _sessionName = "user_session";
    }

    _socket = new QTcpSocket(this);
    _socket->setSocketOption(QTcpSocket::LowDelayOption, 1);
    _socket->setSocketOption(QTcpSocket::KeepAliveOption, 1);

    connect(_socket, SIGNAL(connected()), this, SLOT(_connected()));
    connect(_socket, SIGNAL(disconnected()), this, SLOT(_disconnected()));
    connect(_socket, SIGNAL(readyRead()), this, SLOT(_readyRead()));
    connect(_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(_bytesSent(qint64)));
    connect(_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(_error(QAbstractSocket::SocketError)));

    loadSession();
}

TgTransport::~TgTransport()
{
    stop();
}

void TgTransport::resetSession()
{
    kgDebug() << "Resetting session";

    stop();

    resetDc();
    authKey = QByteArray();
    sessionId = 0;
    userId = 0;

    saveSession();

    pendingMessages.clear();
    migrationMessages.clear();
    msgsToAck.clear();

    initialized = false;

    _client->handleDisconnected();
}

bool TgTransport::hasSession()
{
    return userId && sessionId && serverSalt && authKeyId && !authKey.isEmpty();
}

bool TgTransport::hasUserId()
{
    return userId != 0;
}

TgLong TgTransport::getUserId()
{
    return userId;
}

qint32 TgTransport::dcId()
{
    return currentDc;
}

void TgTransport::saveSession()
{
    if (currentDc == 0) {
        return;
    }

    kgDebug() << "Saving session" << _sessionName;

    //TODO: password encryption
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName() + "_" + _sessionName);

    if (isMain) {
        settings.beginGroup("Transport");
        settings.setValue("MainDc", currentDc);
        settings.endGroup();
    }

    settings.beginGroup("DcSession" + QString::number((currentDc + (testMode ? 10000 : 0)) * (mediaOnly ? -1 : 1)));

    settings.setValue("CurrentHost",    currentHost);
    settings.setValue("CurrentPort",    currentPort);
    settings.setValue("AuthKey",        authKey);
    settings.setValue("ServerSalt",     serverSalt);
    settings.setValue("AuthKeyId",      authKeyId);
    settings.setValue("TimeOffset",     timeOffset);
    settings.setValue("Sequence",       sequence);
    settings.setValue("LastMessageId",  lastMessageId);
    settings.setValue("SessionId",      sessionId);
    settings.setValue("PingId",         pingId);
    settings.setValue("UserId",         userId);

    settings.endGroup();
}

void TgTransport::loadSession()
{
    kgDebug() << "Loading session" << _sessionName;

    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName() + "_" + _sessionName);

    if (isMain) {
        settings.beginGroup("Transport");
        currentDc = settings.value("MainDc").toInt();
        settings.endGroup();
    }

    settings.beginGroup("DcSession" + QString::number((currentDc + (testMode ? 10000 : 0)) * (mediaOnly ? -1 : 1)));

    currentHost     = settings.value("CurrentHost").toString();
    currentPort     = settings.value("CurrentPort").toInt();
    authKey         = settings.value("AuthKey").toByteArray();
    serverSalt      = settings.value("ServerSalt").toLongLong();
    authKeyId       = settings.value("AuthKeyId").toLongLong();
    timeOffset      = settings.value("TimeOffset").toInt();
    sequence        = settings.value("Sequence").toInt();
    lastMessageId   = settings.value("LastMessageId").toLongLong();
    sessionId       = settings.value("SessionId").toLongLong();
    pingId          = settings.value("PingId").toLongLong();
    userId          = settings.value("UserId").toLongLong();

    settings.endGroup();
}

void TgTransport::setDc(QString host, quint16 port, qint32 dcId)
{
    stop();

    currentDc = dcId;
    loadSession();

    currentDc = dcId;
    currentHost = host;
    currentPort = port;
    tgConfig = TgObject();

    saveSession();
}

void TgTransport::resetDc()
{
    setDc(testMode ? "149.154.167.40" : "149.154.167.50", 443, 2);
}

TgObject TgTransport::config()
{
    return tgConfig;
}

void TgTransport::setConfig(TgObject config)
{
    if (!tgConfig.isEmpty()) {
        return;
    }

    tgConfig = config;
}

void TgTransport::migrateTo(qint32 dcId)
{
    if (dcId == currentDc) {
        kgDebug() << "Already on" << currentDc << ", migration not needed";
        return;
    }

    kgDebug() << "Migrating to DC" << dcId;

    stop();

    TgList dcOptions = tgConfig["dc_options"].toList();

    QString targetHost;
    quint16 targetPort;

    for (qint32 i = 0; i < dcOptions.size(); ++i) {
        TgObject option = dcOptions[i].toMap();

        if (option["id"].toInt() != dcId
                || option["ipv6"].toBool()
                || option["media_only"].toBool() != mediaOnly
                || option["tcpo_only"].toBool()
                || option["cdn"].toBool())
            continue;

        targetHost = option["ip_address"].toString();
        targetPort = option["port"].toInt();
        break;
    }

    if (targetHost.isEmpty()) {
        kgDebug() << "DC" << dcId << "not found. Resetting to default DC.";
        resetDc();
        start();
        return;
    }

    currentDc = dcId;
    currentHost = targetHost;
    if (targetPort) {
        currentPort = targetPort;
    }

    start();
}

void TgTransport::start()
{
    if (_socket->state() != 0)
        return;

    loadSession();

    if (currentHost.isEmpty() || !currentPort || !currentDc) {
        resetDc();
        saveSession();
    }

    kgDebug() << "Starting transport, DC" << currentDc;

    _socket->connectToHost(currentHost, currentPort);
    _timer.start(60000, this);
}

void TgTransport::stop(bool sendMsgsAckBool)
{
    saveSession();

    if (_socket->state() == 0)
        return;

    kgDebug() << "Stopping transport";

    if (sendMsgsAckBool)
        sendMsgsAck();

    _timer.stop();
    _socket->flush();
    _socket->disconnectFromHost();
    if (_socket->state() == QTcpSocket::ClosingState)
        _socket->waitForBytesWritten();
}

void TgTransport::_error(QAbstractSocket::SocketError socketError)
{
    kgDebug() << "Socket errored:" << socketError;
}

void TgTransport::_connected()
{
    kgDebug() << "Socket connected";

    TgPacket packet;
    writeInt32(packet, 0xeeeeeeee);

    _socket->write(packet.toByteArray());

    _client->handleConnected();

    if (hasSession()) {
        initConnection();
    } else {
        authorize();
    }
}

void TgTransport::_disconnected()
{
    kgDebug() << "Socket disconnected";

    initialized = false;

    _client->handleDisconnected();
}

void TgTransport::timerEvent(QTimerEvent *event)
{
    kgDebug() << "Sending ping";

    if (pendingMessages.size() >= 40) {
        kgDebug() << "TOO MANY pending messages," << pendingMessages.size() << "items, clearing";
        pendingMessages.clear();
    }

    sendMsgsAck();

    saveSession();

    TGOBJECT(MTType::PingDelayDisconnectMethod, ping);
    ping["ping_id"] = pingId++;
    ping["disconnect_delay"] = 75;

    sendMTObject<&writeMTMethodPingDelayDisconnect>(ping);
}

qint64 TgTransport::sendPlainMessage(QByteArray data, qint64 oldMid)
{
    //TODO: lock

    if (data.isEmpty())
        return 0;

    TgPacket packet;
    writeInt64(packet, 0);
    qint64 messageId = getNewMessageId();
    writeInt64(packet, messageId);
    writeInt32(packet, data.length());

    if (oldMid)
        broadcastMessageChange(oldMid, messageId);

    sendIntermediate(packet.toByteArray() + data);
    return messageId;
}

void TgTransport::sendIntermediate(QByteArray data)
{
    TgPacket packet;
    writeInt32(packet, data.length());

    _socket->write(packet.toByteArray() + data);
}

void TgTransport::authorize()
{
    kgDebug() << "DH exchange: step 1";

    newNonce.clear();
    serverNonce.clear();

    TGOBJECT(MTType::ReqPqMultiMethod, reqPq);
    reqPq["nonce"] = nonce = randomBytes(INT128_BYTES);
    sendPlainObject<&writeMTMethodReqPqMulti>(reqPq);
}

void TgTransport::_readyRead()
{
    //kgDebug() << "Ready to read";

    while (_socket->bytesAvailable()) {
        processMessage(readIntermediate());
    }
}

QByteArray TgTransport::readIntermediate()
{
    QByteArray buffer = readFully(*_socket, 4);

    if (buffer.isEmpty()) {
        return QByteArray();
    }

    TgPacket packet(buffer);
    QVariant var;
    readInt32(packet, var);

    return readFully(*_socket, var.toInt());
}

void TgTransport::processMessage(QByteArray message)
{
    //TODO: lock

    TgPacket packet(message);

    if (message.length() <= 4) {
        QVariant error;
        readInt32(packet, error);
        kgCritical() << "Got MTProto error:" << error.toInt();
        return;
    }

    QVariant var;
    readInt64(packet, var); //auth_key_id

    qint64 messageId;

    QByteArray data;

    if (var.toLongLong() == 0) {
        readInt64(packet, var); //message_id
        messageId = var.toLongLong();
        readInt32(packet, var); //message_length
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
        messageId = var.toLongLong();
        readInt32(plainMessage, var); //remoteSequence
        readInt32(plainMessage, var); //messageLength
        readRawBytes(plainMessage, data, var.toInt());
    }

    handleObject(data, messageId);
}

void TgTransport::handleObject(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;
    readInt32(packet, var);
    qint32 conId = var.toInt();

    //kgDebug() << "Got an object" << conId << ", msg id" << messageId;

    if (!msgsToAck.contains(messageId))
        msgsToAck.append(messageId);

    if (conId != MTType::RpcError && conId != MTType::BadMsgNotification && conId != MTType::BadServerSalt)
        pendingMessages.remove(messageId);

    switch (conId) {
    case TLType::BoolTrue:
    case TLType::BoolFalse:
        handleBool(data, messageId);
        break;
    case MTType::ResPQ:
        handleResPQ(data, messageId);
        break;
    case MTType::ServerDHParamsOk:
        handleServerDHParamsOk(data, messageId);
        break;
    case MTType::DhGenOk:
        handleDhGenOk(data, messageId);
        break;
    case MTType::MsgContainer:
        handleMsgContainer(data, messageId);
        break;
    case MTType::MsgCopy:
        handleMsgCopy(data, messageId);
        break;
    case MTType::RpcResult:
        handleRpcResult(data, messageId);
        break;
    case MTType::GzipPacked:
        handleGzipPacked(data, messageId);
        break;
    case MTType::RpcError:
        handleRpcError(data, messageId);
        break;
    case MTType::BadMsgNotification:
        handleBadMsgNotification(data, messageId);
        break;
    case MTType::BadServerSalt:
        handleBadServerSalt(data, messageId);
        break;
    case MTType::PingMethod:
        handlePingMethod(data, messageId);
        break;
    case TLType::Config:
        handleConfig(data, messageId);
        break;
    case TLType::AuthAuthorization:
        handleAuthorization(data, messageId);
        break;
    case MTType::Vector:
        handleVector(data, messageId);
        break;
    case MTType::MsgDetailedInfo:
        handleMsgDetailedInfo(data, messageId);
        break;
    case MTType::MsgNewDetailedInfo:
        handleMsgNewDetailedInfo(data, messageId);
        break;
    case MTType::NewSessionCreated:
    case MTType::MsgsAck:
    case MTType::Pong:
    case MTType::RpcAnswerUnknown:
    case MTType::RpcAnswerDroppedRunning:
    case MTType::RpcAnswerDropped:
    case MTType::FutureSalt:
    case MTType::FutureSalts:
        kgDebug() << "INFO: Ignoring" << conId;
        break;
    default:
        _client->handleObject(data, messageId);
        break;
    }
}

//TODO: handle NewSessionCreated?

void TgTransport::handleResPQ(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readMTResPQ(packet, var);
    TgObject obj = var.toMap();

    kgDebug() << "DH exchange: step 2";

    if (obj["nonce"].toByteArray() != nonce) {
        kgCritical() << "SECURITY ERROR: nonces are different";
        return;
    }

    serverNonce = obj["server_nonce"].toByteArray();

    kgDebug() << "DH exchange: step 3";

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
                kgDebug() << "Matched fingerprint: " << keychain[j].fingerprint;
                break;
            }
        }
        if (!matched.isEmpty()) {
            break;
        }
    }

    if (matched.isEmpty()) {
        kgCritical() << "SECURITY ERROR: no suitable keys are found";
        return;
    }

    TGOBJECT(MTType::PQInnerDataDc, pqInnerData);
    pqInnerData["pq"] = pqBytes;
    pqInnerData["p"] = pBytes;
    pqInnerData["q"] = qBytes;
    pqInnerData["nonce"] = nonce;
    pqInnerData["server_nonce"] = serverNonce;
    pqInnerData["new_nonce"] = newNonce = randomBytes(INT256_BYTES);
    //it works without DC ID lol
    pqInnerData["dc"] = (currentDc + (testMode ? 10000 : 0)) * (mediaOnly ? -1 : 1);

    TgPacket pidPacket;
    writeMTPQInnerData(pidPacket, pqInnerData);
    QByteArray pidData = pidPacket.toByteArray();

    if (pidData.length() > 144) {
        kgCritical() << "SECURITY ERROR: pqInnerData is longer that 144 bytes";
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

    kgDebug() << "DH exchange: step 4";
}

void TgTransport::handleServerDHParamsOk(QByteArray data, qint64 messageId)
{
    qint32 localTime = QDateTime::currentDateTimeUtc().toTime_t();

    TgPacket packet(data);
    QVariant var;

    readMTServerDHParams(packet, var);
    TgObject obj = var.toMap();

    kgDebug() << "DH exchange: step 5";

    if (obj["nonce"].toByteArray() != nonce) {
        kgCritical() << "SECURITY ERROR: nonces are different";
        return;
    }

    if (obj["server_nonce"].toByteArray() != serverNonce) {
        kgCritical() << "SECURITY ERROR: server nonces are different";
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

    sessionId = randomLong();
    QByteArray gaBytes = innerData["g_a"].toByteArray();
    authKey = encryptRSA(gaBytes, dhPrime, bBytes);
    authKey.insert(0, QByteArray(256 - authKey.size(), 0));
    serverSalt = qFromLittleEndian<qint64>((const uchar*) xorArray(newNonce.mid(0, 8), serverNonce.mid(0, 8)).constData());
    authKeyId = qFromLittleEndian<qint64>((const uchar*) hashSHA1(authKey).mid(12, 8).constData());
    timeOffset = innerData["server_time"].toInt() - localTime;
    sequence = 0;
    lastMessageId = 0;

    sendPlainObject<&writeMTMethodSetClientDHParams>(setParams);
}

void TgTransport::handleDhGenOk(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readMTSetClientDHParamsAnswer(packet, var);
    TgObject obj = var.toMap();

    if (obj["nonce"].toByteArray() != nonce) {
        kgCritical() << "SECURITY ERROR: nonces are different";
        return;
    }

    if (obj["server_nonce"].toByteArray() != serverNonce) {
        kgCritical() << "SECURITY ERROR: server nonces are different";
        return;
    }

    QByteArray newNonceHash1 = hashSHA1(newNonce + QByteArray(1, 1) + hashSHA1(authKey).mid(0, 8)).mid(4);
    if (obj["new_nonce_hash1"].toByteArray() != newNonceHash1) {
        kgCritical() << "SECURITY ERROR: new nonce hashes are different";
        return;
    }

    //TODO: important checks
    kgDebug() << "DH completed";

    saveSession();

    initConnection();
}

void TgTransport::initConnection()
{
    kgDebug() << "Initializing connection";

    TGOBJECT(TLType::HelpGetConfigMethod, getConfig);
    TGOBJECT(TLType::InitConnectionMethod, initConnection);

    initConnection["api_id"] = KUTEGRAM_API_ID;
    initConnection["device_model"] = systemName() + "-based device";
    initConnection["system_version"] = systemName();
    QString appVersion = QCoreApplication::applicationVersion();
    if (appVersion.isEmpty())
        appVersion = "1.0.0";
    initConnection["app_version"] = appVersion;
    initConnection["system_lang_code"] = QLocale::system().name().split("_")[0];
    initConnection["lang_pack"] = "";
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
        kgCritical() << "ERROR: Gzip compression error.";
        return data;
    }

    TGOBJECT(MTType::GzipPacked, zipped);
    zipped["packed_data"] = packedData;

    TgPacket packet;
    writeMTObject(packet, zipped);
    return packet.toByteArray();
}

TgLong TgTransport::sendMsgsAck()
{
    if (msgsToAck.isEmpty())
        return 0;

    if (!initialized)
        return 0;

    kgDebug() << "Sending MsgsAck," << msgsToAck.size() << "items";
    //if (msgsToAck.size() <= 20) kgDebug() << msgsToAck;

    //TODO: max 8192 ids

    TGOBJECT(MTType::MsgsAck, msgsAck);
    msgsAck["msg_ids"] = TgList(msgsToAck);
    msgsToAck.clear();

    return sendMTMessage(tlSerialize<&writeMTMsgsAck>(msgsAck), 0, true);
}

qint64 TgTransport::sendMTMessage(QByteArray originalData, qint64 oldMid, bool isMsgsAck)
{
    //TODO: lock

    if (authKey.isEmpty())
        return sendPlainMessage(originalData, oldMid);

    if (originalData.isEmpty())
        return 0;

    if (!isMsgsAck)
        sendMsgsAck();

    QByteArray data = originalData;
    if (data.size() > 255)
        data = gzipPacket(data);

    TgPacket packet;
    writeInt64(packet, serverSalt);
    writeInt64(packet, sessionId);
    qint64 messageId = getNewMessageId();
    writeInt64(packet, messageId);
    writeInt32(packet, generateSequence(!isMsgsAck));
    writeInt32(packet, data.length());
    writeRawBytes(packet, data);
    writeRawBytes(packet, randomBytes((0x7FFFFFF0 - data.length()) % 16 + (randomInt(14) + 2) * 16));

    QByteArray packetBytes = packet.toByteArray();
    QByteArray messageKey = calcMessageKey(authKey, packetBytes);
    QByteArray keyIv;
    QByteArray keyData = calcEncryptionKey(authKey, messageKey, keyIv, true);
    QByteArray cipherText = encryptAES256IGE(packetBytes, keyIv, keyData);

    TgPacket cipherPacket;
    writeInt64(cipherPacket, authKeyId);
    writeRawBytes(cipherPacket, messageKey);
    writeRawBytes(cipherPacket, cipherText);

    if (!isMsgsAck)
        pendingMessages.insert(messageId, originalData);

    if (oldMid)
        broadcastMessageChange(oldMid, messageId);

    sendIntermediate(cipherPacket.toByteArray());
    return messageId;
}

qint64 TgTransport::getNewMessageId()
{
    //TODO: lock
    qint64 time = QDateTime::currentDateTime().toUTC().toTime_t();
    qint64 newMessageId = ((time + timeOffset) << 32) | (((time + timeOffset) % 1000) << 22);
    if (lastMessageId >= newMessageId) newMessageId = lastMessageId + 4;
    lastMessageId = newMessageId;
    return newMessageId;
}

qint32 TgTransport::generateSequence(bool isContent)
{
    //TODO: lock
    qint32 result = isContent ? sequence++ * 2 + 1 : sequence++ * 2;
    return result;
}

void TgTransport::_bytesSent(qint64 count)
{
    //kgDebug() << "Sent" << count << "bytes";
}

void TgTransport::handleMsgContainer(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readInt32(packet, var); //conId
    readInt32(packet, var); //size
    qint32 size = var.toInt();
    for (qint32 i = 0; i < size; ++i) {
        readInt64(packet, var); //msgId
        qint64 newMessageId = var.toLongLong();
        readInt32(packet, var); //seqNo
        readInt32(packet, var); //size
        QByteArray array;
        readRawBytes(packet, array, var.toInt());
        handleObject(array, newMessageId);
    }
}

void TgTransport::handleRpcResult(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readInt32(packet, var); //conId
    readInt64(packet, var); //req_msg_id

    handleObject(data.mid(12), var.toLongLong());
}

void TgTransport::handleGzipPacked(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readInt32(packet, var); //conId
    readByteArray(packet, var); //gzippedData

    if (!QCompressor::gzipDecompress(var.toByteArray(), data)) {
        kgCritical() << "ERROR: Gzip decompression error.";
        return;
    }

    handleObject(data, messageId);
}

void TgTransport::handleRpcError(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant errorCodeVariant;

    readInt32(packet, errorCodeVariant); //conId
    readInt32(packet, errorCodeVariant);

    qint32 errorCode = errorCodeVariant.toInt();

    QVariant errorMessageVariant;
    readString(packet, errorMessageVariant);

    QString errorMessage = errorMessageVariant.toString();

    qint32 messageParameter = 0;
    QStringList splittedMessage = errorMessage.split("_");
    if (!splittedMessage.isEmpty())
        messageParameter = splittedMessage.last().toInt();

    if (errorMessage.contains("_MIGRATE_")) {
        if (errorMessage.startsWith("FILE_")) {
            pendingMessages.remove(messageId);
            _client->migrateFileTo(messageId, messageParameter);
            return;
        }

        migrationMessages.insert(messageId, pendingMessages.take(messageId));
        migrateTo(messageParameter);
        return;
    }

    if (errorCode == 401) {
        resetSession();
        return;
    }

    pendingMessages.remove(messageId);

    _client->handleRpcError(errorCode, errorMessage, messageId);
}

void TgTransport::handlePingMethod(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant pingId;

    readInt32(packet, pingId); //conId
    readInt64(packet, pingId);

    TGOBJECT(MTType::Pong, pong);
    pong["msg_id"] = messageId;
    pong["ping_id"] = pingId.toLongLong();

    sendMTObject<&writeMTPong>(pong);
}

void TgTransport::handleMsgCopy(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readInt32(packet, var); //conId
    readInt64(packet, var); //msgId
    qint64 newMessageId = var.toLongLong();
    readInt32(packet, var); //seqNo
    readInt32(packet, var); //size
    QByteArray array;
    readRawBytes(packet, array, var.toInt());
    handleObject(array, newMessageId);
}

void TgTransport::handleBadMsgNotification(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;

    readInt32(packet, var); //conId
    readInt64(packet, var); //badMsgId
    qint64 badMsgId = var.toLongLong();
    readInt32(packet, var); //badMsgSeqNo
    readInt32(packet, var); //errorCode
    qint32 errorCode = var.toInt();

    switch (errorCode) {
    case 16:
    case 17:
        lastMessageId = 0;
        timeOffset = (qint32) (messageId >> 32) - QDateTime::currentDateTimeUtc().toTime_t();
        break;
    case 32:
    case 33:
        lastMessageId = 0;
        sequence = 0;
        sessionId = randomLong();
        break;
    case 48:
        readInt64(packet, var);
        serverSalt = var.toLongLong();
        break;
    }

    //TODO: fix too many bad msg 35 errors (fix MsgsAck?)

    saveSession();

    sendMTMessage(pendingMessages.take(badMsgId), badMsgId, false);

    kgDebug() << "INFO: bad msg notification handled" << errorCode;
}

//TODO: flood wait

void TgTransport::handleBadServerSalt(QByteArray data, qint64 messageId)
{
    handleBadMsgNotification(data, messageId);
}

void TgTransport::handleConfig(QByteArray data, qint64 messageId)
{
    tgConfig = tlDeserialize<&readTLConfig>(data).toMap();

    qint32 thisDc = tgConfig["this_dc"].toInt();
    if (thisDc)
        currentDc = thisDc;

    saveSession();

    QList<qint64> keys = migrationMessages.keys();
    kgDebug() << "Pending:" << keys;
    for (qint32 i = 0; i < keys.size(); ++i) {
        sendMTMessage(migrationMessages.take(keys[i]), keys[i], false);
    }

    initialized = true;

    _client->handleInitialized();

    checkAuthorization();
}

void TgTransport::checkAuthorization()
{
    if (!userId)
        return;

    kgDebug() << "Checking authorzation";

    TGOBJECT(TLType::UsersGetUsersMethod, getUsers);
    TGOBJECT(TLType::InputUserSelf, self);
    TgList ids;
    ids.append(self);
    getUsers["id"] = ids;

    authCheckMsgId = sendMTObject<&writeTLMethodUsersGetUsers>(getUsers);
}

void TgTransport::handleVector(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    TgVariant var;

    readInt32(packet, var);
    readInt32(packet, var);

    TgObject user;

    if (var.toInt() > 0) {
        readTLUser(packet, var);
        user = var.toMap();
    }

    qint64 newUserId = user["id"].toLongLong();

    if (messageId == authCheckMsgId && GETID(user) != 0 && isUser(user) && newUserId) {
        userId = newUserId;

        saveSession();

        _client->handleAuthorized(userId);
    }
//    else if (GETID(user) != 0 && isUser(user))
//        kgDebug() << "Got user vector, msgsIds:" << messageId << authCheckMsgId;

    _client->handleObject(data, messageId);
}

void TgTransport::handleAuthorization(QByteArray data, qint64 messageId)
{
    TgObject auth = tlDeserialize<&readTLAuthAuthorization>(data).toMap();

    userId = auth["user"].toMap()["id"].toLongLong();

    saveSession();

    checkAuthorization();

    _client->handleObject(data, messageId);
}

void TgTransport::handleMsgDetailedInfo(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    TgVariant var;

    readInt32(packet, var); //conId
    readInt64(packet, var); //msgId

    if (!msgsToAck.contains(var.toLongLong()))
        msgsToAck.append(var.toLongLong());

    readInt64(packet, var); //answerMsgId

    if (!msgsToAck.contains(var.toLongLong()))
        msgsToAck.append(var.toLongLong());
}

void TgTransport::broadcastMessageChange(qint64 oldMsg, qint64 newMsg)
{
    if (!oldMsg)
        return;

    if (authCheckMsgId == oldMsg)
        authCheckMsgId = newMsg;

    _client->handleMessageChanged(oldMsg, newMsg);
}

void TgTransport::handleMsgNewDetailedInfo(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    TgVariant var;

    readInt32(packet, var); //conId
    readInt64(packet, var); //answerMsgId

    if (!msgsToAck.contains(var.toLongLong()))
        msgsToAck.append(var.toLongLong());
}

void TgTransport::handleBool(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    TgVariant var;

    readInt32(packet, var); //conId

    _client->handleBool(var.toInt() == TLType::BoolTrue, messageId);
}
