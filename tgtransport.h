#ifndef TGTRANSPORT_H
#define TGTRANSPORT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include "tgstream.h"

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

    QByteArray authKey;
    quint64 serverSalt;
    quint64 authKeyId;
    qint32 timeOffset;
    qint32 sequence;
    qint64 lastMessageId;
    quint64 sessionId;

public:
    explicit TgTransport(TgClient *parent = 0);
    template <WRITE_METHOD W> void sendPlainObject(QVariant i);
    template <WRITE_METHOD W> void sendMTObject(QVariant i);

signals:
    
public slots:
    void sendPlainMessage(QByteArray data);
    void sendMTMessage(QByteArray data);
    void authorize();
    void sendIntermediate(QByteArray data);
    QByteArray readIntermediate();
    void processMessage(QByteArray message);
    void initConnection();
    void handleObject(QByteArray data);
    void handleResPQ(QByteArray data);
    void handleServerDHParamsOk(QByteArray data);
    void handleDhGenOk(QByteArray data);
    QByteArray gzipPacket(QByteArray data);
    void _readyRead();
    void _bytesSent(qint64 count);
    qint64 getNewMessageId();
    qint32 generateSequence(bool isContent);
};

template <WRITE_METHOD W> void TgTransport::sendPlainObject(QVariant i)
{
    QByteArray serialized = tlSerialize<W>(i);
    //qDebug() << "[OUT]" << serialized.toHex();
    sendPlainMessage(serialized);
}

template <WRITE_METHOD W> void TgTransport::sendMTObject(QVariant i)
{
    QByteArray serialized = tlSerialize<W>(i);
    //qDebug() << "[OUT]" << serialized.toHex();
    sendMTMessage(serialized);
}

#endif // TGTRANSPORT_H
