#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QDebug>
#include "tgtransport.h"
#include "tlschema.h"

TgClient::TgClient(QObject *parent)
    : QObject(parent)
    , _transport(0)
{

}

void TgClient::start()
{
    qDebug() << "Starting client";

    _transport = new TgTransport(this);
    _transport->start();
}

void TgClient::handleConnected()
{
    qDebug() << "Client connected";
    emit connected();
}

void TgClient::handleDisconnected()
{
    qDebug() << "Client disconnected";
    emit disconnected();
}

void TgClient::handleInitialized()
{
    qDebug() << "Client initialized";
    emit initialized();
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
    default:
        qDebug() << "INFO: Unhandled object " << conId;
        break;
    }
}
