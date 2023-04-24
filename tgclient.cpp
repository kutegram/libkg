#include "tgclient.h"

#include <QNetworkAccessManager>
#include <QDebug>
#include "tgtransport.h"

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
