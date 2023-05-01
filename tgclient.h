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
    void stop();

    void handleObject(QByteArray data, qint64 messageId);

    void handleConnected();
    void handleDisconnected();
    void handleInitialized();
    void handleRpcError(qint32 errorCode, QString errorMessage);

    qint64 helpGetCountriesList(qint32 hash = 0, QString langCode = "");
    qint64 authSendCode(QString phoneNumber);
    qint64 authSignIn(QString phoneNumber, QString phoneCodeHash, QString phoneCode);

signals:
    void connected();
    void disconnected();
    void initialized();
    void rpcError(qint32 errorCode, QString errorMessage);

    void helpGetCountriesListResponse(QVariantMap object, qint64 messageId);
    void authSendCodeResponse(QVariantMap object, qint64 messageId);
    void authSignInResponse(QVariantMap object, qint64 messageId);

};

template <WRITE_METHOD W> qint64 TgClient::sendObject(QVariant i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
