#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QStringList>
#include "debug.h"
#include "tgtransport.h"
#include "tlschema.h"
#include <QDir>
#include <QSettings>
#include <QCoreApplication>

//TODO: isNull in schema to toBool?

TgClient::TgClient(QObject *parent, qint32 dcId, QString sessionName)
    : QObject(parent)
    , clientSessionName()
    , _transport(0)
    , processedFiles()
    , processedDownloadFiles()
    , currentDownloading(0)
    , filePackets()
    , clientForDc()
    , _main()
    , _connected()
    , _initialized()
    , _authorized()
    , migrationForDc()
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

TgLong TgClient::getUserId()
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

