#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include "tgtransport.h"

class TgClient : public QObject
{
    Q_OBJECT
private:
    TgTransport *_transport;

public:
    explicit TgClient(QObject *parent = 0);
    
    template <WRITE_METHOD W> qint64 sendObject(QVariant i);
    
public slots:
    void start();
    void handleObject(QByteArray data, qint64 messageId);

    void handleConnected();
    void handleDisconnected();
    void handleInitialized();

    qint64 helpGetCountriesList(qint32 hash = 0, QString langCode = "");

signals:
    void connected();
    void disconnected();
    void initialized();

    void helpGetCountriesListResponse(QVariantMap object, qint64 messageId);

};

template <WRITE_METHOD W> qint64 TgClient::sendObject(QVariant i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
