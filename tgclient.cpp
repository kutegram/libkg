#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QStringList>
#include "debug.h"
#include "tgtransport.h"
#include "tlschema.h"

//TODO: isNull in schema to toBool?

TgClient::TgClient(QObject *parent, qint32 dcId, QString sessionName)
    : QObject(parent)
    , clientSessionName(sessionName)
    , _transport(new TgTransport(this, sessionName, dcId))
    , processedFiles()
    , processedDownloadFiles()
    , filePackets()
    , clientForDc()
    , isMain(dcId == 0)
{

}

TgClient* TgClient::getClientForDc(int dcId)
{
    if (!isMain) {
        TgClient* c = static_cast<TgClient*>(parent());
        if (c == 0) {
            return 0;
        }
        return c->getClientForDc(dcId);
    }

    kgInfo() << "Requested client with DC ID" << dcId;

    if (dcId == 0 || _transport->dcId() == dcId) {
        return this;
    }

    TgClient* client = clientForDc.value(dcId);

    if (client != 0) {
        return client;
    }

    client = new TgClient(this, dcId, clientSessionName);
    clientForDc.insert(dcId, client);
    client->migrateTo(_transport->config(), dcId);

    //TODO: migrate authorization

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

void TgClient::handleConnected()
{
    kgDebug() << "Client connected";

    clientForDc.insert(_transport->dcId(), this);

    emit connected(hasUserId());
}

void TgClient::handleDisconnected()
{
    kgDebug() << "Client disconnected";

    emit disconnected(hasUserId());
}

void TgClient::handleInitialized()
{
    kgDebug() << "Client initialized";

    emit initialized(hasUserId());
}

void TgClient::handleAuthorized(qint64 userId)
{
    kgDebug() << "Client authorized";

    if (userId != 0) {
        QList<TgLong> fileIds = processedDownloadFiles.keys();
        for (qint32 i = 0; i < fileIds.size(); ++i) {
            downloadNextFilePart(fileIds[i]);
        }
    }

    emit authorized(userId);
}

void TgClient::handleMessageChanged(qint64 oldMsg, qint64 newMsg)
{
    kgDebug() << "Message changed" << oldMsg << "->" << newMsg;

    TgLong fileId = filePackets.take(oldMsg);
    if (fileId != 0) {
        filePackets.insert(newMsg, fileId);
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
    default:
        kgDebug() << "UNHANDLED object:" << conId;
        emit unknownResponse(conId, data, messageId);
        break;
    }
}
