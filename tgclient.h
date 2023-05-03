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
    
    template <WRITE_METHOD W> TgLong sendObject(TgObject i);
    
public slots:
    void start();
    void stop();

    void handleObject(QByteArray data, qint64 messageId);

    void handleConnected();
    void handleDisconnected();
    void handleInitialized();
    void handleRpcError(qint32 errorCode, QString errorMessage);

    TgLong helpGetCountriesList(qint32 hash = 0, QString langCode = "");
    TgLong authSendCode(QString phoneNumber);
    TgLong authSignIn(QString phoneNumber, QString phoneCodeHash, QString phoneCode);

signals:
    void connected();
    void disconnected();
    void initialized();
    void rpcError(qint32 errorCode, QString errorMessage);

    void helpGetCountriesListResponse(TgObject object, TgLong messageId);
    void authSendCodeResponse(TgObject object, TgLong messageId);
    void authSignInResponse(TgObject object, TgLong messageId);

};

template <WRITE_METHOD W> TgLong TgClient::sendObject(TgObject i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
