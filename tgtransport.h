#ifndef TGTRANSPORT_H
#define TGTRANSPORT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include "telegramstream.h"

class TgClient;

class TgTransport : public QObject
{
    Q_OBJECT
private:
    TgClient *_client;
    QTcpSocket *_socket;

    QByteArray nonce;
    QByteArray serverNonce;
    QByteArray newNonce;

public:
    explicit TgTransport(TgClient *parent = 0);
    template <WRITE_METHOD W> void sendPlainObject(QVariant i);

signals:
    
public slots:
    void sendPlainMessage(QByteArray data);
    void authorize();
    void sendIntermediate(QByteArray data);
    void readIntermediate(QByteArray &data);
    void processMessage();

    void handleObject(qint32 conId, QByteArray data);
    void handleResPQ(QByteArray data);
    void handleServerDHParamsOk(QByteArray data);

    void _readyRead();
    void _bytesSent(qint64 count);
};

template <WRITE_METHOD W> void TgTransport::sendPlainObject(QVariant i)
{
    QByteArray serialized = tlSerialize<W>(i);
    //qDebug() << "[OUT]" << serialized.toHex();
    sendPlainMessage(serialized);
}

#endif // TGTRANSPORT_H
