#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QDebug>
#include "tgtransport.h"

TgClient::TgClient(QObject *parent)
    : QObject(parent)
    , _socket(0)
    , _transport(0)
{
}

void TgClient::start()
{
    qDebug() << "Starting client";

    _socket = new QTcpSocket(this);
    _socket->setSocketOption(QTcpSocket::LowDelayOption, 1);
    _socket->setSocketOption(QTcpSocket::KeepAliveOption, 1);
    //TODO: remove hardcode
    _socket->connectToHost("149.154.167.40", 443);

    connect(_socket, SIGNAL(connected()), this, SLOT(_connected()));
}

QTcpSocket* TgClient::socket()
{
    return _socket;
}

void TgClient::_connected()
{
    qDebug() << "Socket connected";

    _transport = new TgTransport(this);
    _transport->authorize();
}
