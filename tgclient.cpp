#include "tgclient.h"

#include <QNetworkAccessManager>
#include "debug.h"
#include "tgtransport.h"
#include "tlschema.h"

TgClient::TgClient(QObject *parent)
    : QObject(parent)
    , _transport(0)
{

}

void TgClient::start()
{
    if (_transport)
        return;

    kgDebug() << "Starting client";

    _transport = new TgTransport(this);
    _transport->start();
}

void TgClient::stop() {
    if (!_transport)
        return;

    kgDebug() << "Stopping client";

    _transport->stop();
    _transport->deleteLater();
    _transport = 0;
}

void TgClient::handleConnected()
{
    kgDebug() << "Client connected";
    emit connected();
}

void TgClient::handleDisconnected()
{
    kgDebug() << "Client disconnected";
    emit disconnected();
}

void TgClient::handleInitialized()
{
    kgDebug() << "Client initialized";
    emit initialized();
}

void TgClient::handleRpcError(qint32 errorCode, QString errorMessage)
{
    kgWarning() << "RPC:" << errorCode << ":" << errorMessage;
    emit rpcError(errorCode, errorMessage);
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
    default:
        kgDebug() << "INFO: Unhandled object " << conId;
        break;
    }
}
