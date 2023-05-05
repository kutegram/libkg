#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QStringList>
#include "debug.h"
#include "tgtransport.h"
#include "tlschema.h"

TgClient::TgClient(QObject *parent, QString sessionName)
    : QObject(parent)
    , _transport(new TgTransport(this, sessionName))
{

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

    emit authorized(userId);
}

void TgClient::handleRpcError(qint32 errorCode, QString errorMessage, qint64 messageId)
{
    kgWarning() << "RPC:" << errorCode << ":" << errorMessage;

    emit rpcError(errorCode, errorMessage, messageId);
}

void TgClient::handleObject(QByteArray data, qint64 messageId)
{
    TgPacket packet(data);
    QVariant var;
    readInt32(packet, var);
    qint32 conId = var.toInt();

    //TODO: map

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
    default:
        kgDebug() << "INFO: Unhandled object " << conId;
        break;
    }
}
