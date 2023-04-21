#ifndef TGTRANSPORT_H
#define TGTRANSPORT_H

#include <QObject>
#include <QTcpSocket>
#include <QBasicTimer>
#include <QDebug>
#include "tgstream.h"

class TgClient;

class TgTransport : public QObject
{
    Q_OBJECT
private:
    TgClient *_client;
    QTcpSocket *_socket;
    QBasicTimer _timer;

    QByteArray nonce;
    QByteArray serverNonce;
    QByteArray newNonce;

    QByteArray authKey;
    qint64 serverSalt;
    qint64 authKeyId;
    qint32 timeOffset;
    qint32 sequence;
    qint64 lastMessageId;
    qint64 sessionId;
    qint64 pingId;

public:
    explicit TgTransport(TgClient *parent = 0);
    template <WRITE_METHOD W> void sendPlainObject(QVariant i);
    template <WRITE_METHOD W> void sendMTObject(QVariant i);
    void timerEvent(QTimerEvent *event);

signals:
    
public slots:
    void sendPlainMessage(QByteArray data);
    void sendMTMessage(QByteArray data);
    void authorize();
    void sendIntermediate(QByteArray data);
    QByteArray readIntermediate();
    void processMessage(QByteArray message);
    void initConnection();
    QByteArray gzipPacket(QByteArray data);
    void _readyRead();
    void _bytesSent(qint64 count);
    qint64 getNewMessageId();
    qint32 generateSequence(bool isContent);

    void handleObject(QByteArray data, qint64 messageId);
    void handleResPQ(QByteArray data, qint64 messageId);
    void handleServerDHParamsOk(QByteArray data, qint64 messageId);
    void handleDhGenOk(QByteArray data, qint64 messageId);
    void handleMsgContainer(QByteArray data, qint64 messageId);
    void handleRpcResult(QByteArray data, qint64 messageId);
    void handleGzipPacked(QByteArray data, qint64 messageId);
    void handleRpcError(QByteArray data, qint64 messageId);
    void handlePingMethod(QByteArray data, qint64 messageId);
    void handleMsgCopy(QByteArray data, qint64 messageId);
    void handleBadMsgNotification(QByteArray data, qint64 messageId);
    void handleBadServerSalt(QByteArray data, qint64 messageId);
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
