#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QStringList>
#include "debug.h"
#include "tgtransport.h"
#include "tlschema.h"
#include <QDir>
#include <QSettings>
#include <QCoreApplication>

#ifdef QT_DECLARATIVE_LIB
#include <QtDeclarative>

void TgClient::registerQML()
{
    qmlRegisterType<TgClient>("Kutegram", 1, 0, "TgClient");
    qmlRegisterType<TgStream>("Kutegram", 1, 0, "TgStream");
    qmlRegisterType<TgPacket>("Kutegram", 1, 0, "TgPacket");
    qRegisterMetaType<TelegramObject>("TelegramObject");
    qRegisterMetaType<TgVariant>("TgVariant");
    qRegisterMetaType<TgObject>("TgObject");
    qRegisterMetaType<TgMap>("TgMap");
    qRegisterMetaType<TgVector>("TgVector");
    qRegisterMetaType<TgList>("TgList");
    qRegisterMetaType<TgArray>("TgArray");
    qRegisterMetaType<TgInt128>("TgInt128");
    qRegisterMetaType<TgInt256>("TgInt256");
    qRegisterMetaType<TgInt>("TgInt");
    qRegisterMetaType<TgInteger>("TgInteger");
    qRegisterMetaType<TgLong>("TgLong");
    qRegisterMetaType<TgLongVariant>("TgLongVariant");
    qRegisterMetaType<TgDouble>("TgDouble");
    qRegisterMetaType<TgString>("TgString");
    qRegisterMetaType<TgBool>("TgBool");
    qRegisterMetaType<TgByteArray>("TgByteArray");
}
#else
void TgClient::registerQML()
{
    kgWarning() << "QtDeclarative is not detected. Ignoring.";
}
#endif

TgClient::TgClient(QObject *parent, qint32 dcId, QString sessionName)
    : QObject(parent)
    , clientSessionName()
    , _transport(0)
    , processedFiles()
    , processedDownloadFiles()
    , currentDownloading(0)
    , filePackets()
    , clientForDc()
    , migrationForDc()
    , _main()
    , _connected()
    , _initialized()
    , _authorized()
    , importMethod()
    , _cacheDirectory()
{
    _main = dcId == 0;
    clientSessionName = sessionName;
    if (clientSessionName.isEmpty()) {
        clientSessionName = "user_session";
    }

    _cacheDirectory = QDir(QDir::cleanPath(QSettings(
                                           QSettings::IniFormat,
                                           QSettings::UserScope,
                                           QCoreApplication::organizationName(),
                                           QCoreApplication::applicationName() + "_" + clientSessionName)
                                       .fileName()+"/../" + QCoreApplication::applicationName() + "_" + clientSessionName));
    _cacheDirectory.mkpath(".");

    _transport = new TgTransport(this, clientSessionName, dcId);
}

qint32 TgClient::getObjectId(TgObject var)
{
    return var["_"].toInt();
}

QByteArray TgClient::getJpegPayload(QByteArray bytes)
{
    if (bytes.size() < 3 || bytes[0] != '\x01') {
        return QByteArray();
    }
    const char header[] = "\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49"
                    "\x46\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00\xff\xdb\x00\x43\x00\x28\x1c"
                    "\x1e\x23\x1e\x19\x28\x23\x21\x23\x2d\x2b\x28\x30\x3c\x64\x41\x3c\x37\x37"
                    "\x3c\x7b\x58\x5d\x49\x64\x91\x80\x99\x96\x8f\x80\x8c\x8a\xa0\xb4\xe6\xc3"
                    "\xa0\xaa\xda\xad\x8a\x8c\xc8\xff\xcb\xda\xee\xf5\xff\xff\xff\x9b\xc1\xff"
                    "\xff\xff\xfa\xff\xe6\xfd\xff\xf8\xff\xdb\x00\x43\x01\x2b\x2d\x2d\x3c\x35"
                    "\x3c\x76\x41\x41\x76\xf8\xa5\x8c\xa5\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
                    "\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
                    "\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
                    "\xf8\xf8\xf8\xf8\xf8\xff\xc0\x00\x11\x08\x00\x00\x00\x00\x03\x01\x22\x00"
                    "\x02\x11\x01\x03\x11\x01\xff\xc4\x00\x1f\x00\x00\x01\x05\x01\x01\x01\x01"
                    "\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"
                    "\x09\x0a\x0b\xff\xc4\x00\xb5\x10\x00\x02\x01\x03\x03\x02\x04\x03\x05\x05"
                    "\x04\x04\x00\x00\x01\x7d\x01\x02\x03\x00\x04\x11\x05\x12\x21\x31\x41\x06"
                    "\x13\x51\x61\x07\x22\x71\x14\x32\x81\x91\xa1\x08\x23\x42\xb1\xc1\x15\x52"
                    "\xd1\xf0\x24\x33\x62\x72\x82\x09\x0a\x16\x17\x18\x19\x1a\x25\x26\x27\x28"
                    "\x29\x2a\x34\x35\x36\x37\x38\x39\x3a\x43\x44\x45\x46\x47\x48\x49\x4a\x53"
                    "\x54\x55\x56\x57\x58\x59\x5a\x63\x64\x65\x66\x67\x68\x69\x6a\x73\x74\x75"
                    "\x76\x77\x78\x79\x7a\x83\x84\x85\x86\x87\x88\x89\x8a\x92\x93\x94\x95\x96"
                    "\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3\xb4\xb5\xb6"
                    "\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4\xd5\xd6"
                    "\xd7\xd8\xd9\xda\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf1\xf2\xf3\xf4"
                    "\xf5\xf6\xf7\xf8\xf9\xfa\xff\xc4\x00\x1f\x01\x00\x03\x01\x01\x01\x01\x01"
                    "\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"
                    "\x09\x0a\x0b\xff\xc4\x00\xb5\x11\x00\x02\x01\x02\x04\x04\x03\x04\x07\x05"
                    "\x04\x04\x00\x01\x02\x77\x00\x01\x02\x03\x11\x04\x05\x21\x31\x06\x12\x41"
                    "\x51\x07\x61\x71\x13\x22\x32\x81\x08\x14\x42\x91\xa1\xb1\xc1\x09\x23\x33"
                    "\x52\xf0\x15\x62\x72\xd1\x0a\x16\x24\x34\xe1\x25\xf1\x17\x18\x19\x1a\x26"
                    "\x27\x28\x29\x2a\x35\x36\x37\x38\x39\x3a\x43\x44\x45\x46\x47\x48\x49\x4a"
                    "\x53\x54\x55\x56\x57\x58\x59\x5a\x63\x64\x65\x66\x67\x68\x69\x6a\x73\x74"
                    "\x75\x76\x77\x78\x79\x7a\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x92\x93\x94"
                    "\x95\x96\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3\xb4"
                    "\xb5\xb6\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4"
                    "\xd5\xd6\xd7\xd8\xd9\xda\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf2\xf3\xf4"
                    "\xf5\xf6\xf7\xf8\xf9\xfa\xff\xda\x00\x0c\x03\x01\x00\x02\x11\x03\x11\x00"
                    "\x3f\x00";
    const char footer[] = "\xff\xd9";
    QByteArray real(header, sizeof(header) - 1);
    real[164] = bytes[1];
    real[166] = bytes[2];
    return real + bytes.mid(3) + QByteArray::fromRawData(footer, sizeof(footer) - 1);
}

QDir TgClient::cacheDirectory()
{
    return _cacheDirectory;
}

TgInt TgClient::dcId()
{
    return _transport->dcId();
}

TgClient* TgClient::getClientForDc(int dcId)
{
    if (!isMain()) {
        TgClient* c = static_cast<TgClient*>(parent());
        if (c == 0) {
            return 0;
        }
        return c->getClientForDc(dcId);
    }

    if (dcId == 0 || _transport->dcId() == dcId) {
        return this;
    }

    kgInfo() << "Requested client with DC ID" << dcId;

    TgClient* client = clientForDc.value(dcId);

    if (client != 0) {
        return client;
    }

    client = new TgClient(this, dcId, clientSessionName);
    clientForDc.insert(dcId, client);
    client->migrateTo(_transport->config(), dcId);

    return client;
}

void TgClient::migrateTo(TgObject config, qint32 dcId)
{
    _transport->setDc("", 443, 0);
    _transport->setConfig(config);
    _transport->migrateTo(dcId);
}

void TgClient::resetSession()
{
    kgDebug() << "Resetting session";

    _transport->resetSession();
}

bool TgClient::hasSession()
{
    return _transport->hasSession();
}

bool TgClient::hasUserId()
{
    return _transport->hasUserId();
}

TgLongVariant TgClient::getUserId()
{
    return _transport->getUserId();
}

void TgClient::start()
{
    kgDebug() << "Starting client";

    _transport->start();
}

void TgClient::stop()
{
    kgDebug() << "Stopping client";

    _transport->stop();
}

bool TgClient::isMain()
{
    return _main;
}

bool TgClient::isConnected()
{
    return _connected;
}

bool TgClient::isInitialized()
{
    return _connected && _initialized;
}

bool TgClient::isAuthorized()
{
    return _connected && _initialized && _authorized;
}

void TgClient::handleDisconnected()
{
    kgDebug() << "Client disconnected";

    _connected = _initialized = _authorized = false;

    emit disconnected(hasUserId());
}

void TgClient::handleTFARequired()
{
    kgDebug() << "Account required TFA to log in";

    emit tfaRequired();
}

void TgClient::handleConnected()
{
    kgDebug() << "Client connected";

    _connected = true;

    clientForDc.insert(_transport->dcId(), this);

    emit connected(hasUserId());
}

void TgClient::handleInitialized()
{
    kgDebug() << "Client initialized";

    _initialized = true;

    if (!importMethod.isEmpty()) {
        sendObject<&writeTLMethodAuthImportAuthorization>(importMethod);
        importMethod = TgObject();
    }

    if (!hasUserId() && !isMain()) {
        TgClient* c = static_cast<TgClient*>(parent());
        if (c != 0) {
            c->migrationForDc.insert(c->exportAuthorization(_transport->dcId()), _transport->dcId());
        }
    }

    emit initialized(hasUserId());
}

void TgClient::handleAuthorized(qint64 userId)
{
    kgDebug() << "Client authorized";

    _authorized = userId != 0;

    if (userId != 0 && !processedDownloadFiles.isEmpty()) {
        downloadNextFilePart();
    }

    emit authorized(userId);
}

void TgClient::handleMessageChanged(qint64 oldMsg, qint64 newMsg)
{
    kgDebug() << "Message changed" << oldMsg << "->" << newMsg;

    migrationForDc.insert(newMsg, migrationForDc.take(oldMsg));

    TgLong fileId = filePackets.take(oldMsg);
    TgFileCtx *uploadCtx = processedFiles.value(fileId, NULL);
    TgFileCtx *downloadCtx = processedDownloadFiles.value(fileId, NULL);
    if (fileId != 0) {
        filePackets.insert(newMsg, fileId);
        if (uploadCtx != NULL) {
            uploadCtx->queue.removeOne(oldMsg);
            uploadCtx->queue.append(newMsg);
        }
        if (downloadCtx != NULL) {
            downloadCtx->queue.removeOne(oldMsg);
            downloadCtx->queue.append(newMsg);
        }
    }

    emit messageChanged(oldMsg, newMsg);
}

void TgClient::handleRpcError(qint32 errorCode, QString errorMessage, qint64 messageId)
{
    if (errorMessage == "OFFSET_INVALID") {
        fileProbablyDownloaded(messageId);
        return;
    }

    if (errorMessage == "FILE_ID_INVALID") {
        cancelDownload(filePackets.take(messageId));
        return;
    }

    kgWarning() << "RPC:" << errorCode << ":" << errorMessage;

    emit rpcError(errorCode, errorMessage, messageId);
}

void TgClient::handleBool(bool response, qint64 messageId)
{
    handleUploadingFile(response, messageId);

    emit boolResponse(response, messageId);
}

void TgClient::handleObject(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;
    readInt32(packet, var);
    qint32 conId = var.toInt();

    switch (conId) {
    case TLType::HelpCountriesList:
    case TLType::HelpCountriesListNotModified:
        emit helpGetCountriesListResponse(tlDeserialize<&readTLHelpCountriesList>(data).toMap(), messageId);
        break;
    case TLType::AuthSentCode:
    case TLType::AuthSentCodeSuccess:
        emit authSendCodeResponse(tlDeserialize<&readTLAuthSentCode>(data).toMap(), messageId);
        break;
    case TLType::AuthAuthorization:
    case TLType::AuthAuthorizationSignUpRequired:
        emit authSignInResponse(tlDeserialize<&readTLAuthAuthorization>(data).toMap(), messageId);
        break;
    case TLType::MessagesDialogs:
    case TLType::MessagesDialogsNotModified:
    case TLType::MessagesDialogsSlice:
        emit messagesGetDialogsResponse(tlDeserialize<&readTLMessagesDialogs>(data).toMap(), messageId);
        break;
    case TLType::UploadFile:
    case TLType::UploadFileCdnRedirect:
        handleUploadFile(tlDeserialize<&readTLUploadFile>(data).toMap(), messageId);
        break;
    case TLType::UpdateShort:
    case TLType::Updates:
        //TODO: handle all types of updates
        break;
    case TLType::AuthExportedAuthorization:
    {
        TgObject exported = tlDeserialize<&readTLAuthExportedAuthorization>(data).toMap();
        TgInt dcId = migrationForDc.take(messageId);
        TgClient* c = getClientForDc(dcId);
        if (dcId != 0 && c != 0) {
            c->importAuthorization(exported["id"].toLongLong(), exported["bytes"].toByteArray());
        }
        break;
    }
    case TLType::MessagesChannelMessages:
    case TLType::MessagesMessages:
    case TLType::MessagesMessagesSlice:
    case TLType::MessagesMessagesNotModified:
        emit messagesGetHistoryResponse(tlDeserialize<&readTLMessagesMessages>(data).toMap(), messageId);
        break;
    default:
        kgDebug() << "UNHANDLED object:" << conId;
        emit unknownResponse(conId, data, messageId);
        break;
    }
}

TgLong TgClient::exportAuthorization(qint32 dcId)
{
    TGOBJECT(TLType::AuthExportAuthorizationMethod, method);

    method["dc_id"] = dcId;

    return sendObject<&writeTLMethodAuthExportAuthorization>(method);
}

TgLong TgClient::importAuthorization(qint64 id, QByteArray bytes)
{
    kgInfo() << "Importing authorization" << id;

    TGOBJECT(TLType::AuthImportAuthorizationMethod, method);

    method["id"] = id;
    method["bytes"] = bytes;

    if (!isInitialized()) {
        importMethod = method;
        return 0;
    }

    return sendObject<&writeTLMethodAuthImportAuthorization>(method);
}
