#ifndef TGTRANSPORT_H
#define TGTRANSPORT_H

#include <QObject>
#include <QTcpSocket>
#include <QBasicTimer>
#include "debug.h"
#include "tgstream.h"

class TgClient;

class TgTransport : public QObject
{
    Q_OBJECT
private:
    TgClient *_client;
    QTcpSocket *_socket;
    QBasicTimer _timer;

    bool testMode;
    bool mediaOnly;
    qint32 currentDc;
    quint16 currentPort;
    QString currentHost;

    TgObject tgConfig;

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
    qint64 userId;

    QMap<qint64, QByteArray> pendingMessages;

    QString _sessionName;

    TgVector msgsToAck;

    qint64 authCheckMsgId;

public:
    explicit TgTransport(TgClient *parent = 0, QString sessionName = "");
    ~TgTransport();

    template <WRITE_METHOD W> qint64 sendPlainObject(QVariant i);
    template <WRITE_METHOD W> qint64 sendMTObject(QVariant i);

signals:
    
public slots:
    void timerEvent(QTimerEvent *event);

    void resetSession();
    void saveSession();
    void loadSession();

    bool hasSession();
    bool hasUserId();

    void checkAuthorization();

    TgLong sendMsgsAck();

    void migrateTo(qint32 dcId);
    void resetDc();

    void start();
    void stop();

    qint64 sendPlainMessage(QByteArray data);
    qint64 sendMTMessage(QByteArray data);
    void authorize();
    void sendIntermediate(QByteArray data);
    QByteArray readIntermediate();
    void processMessage(QByteArray message);
    void initConnection();
    QByteArray gzipPacket(QByteArray data);
    qint64 getNewMessageId();
    qint32 generateSequence(bool isContent);

    void _connected();
    void _disconnected();
    void _readyRead();
    void _bytesSent(qint64 count);

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
    void handleConfig(QByteArray data, qint64 messageId);
    void handleAuthorization(QByteArray data, qint64 messageId);
    void handleVector(QByteArray data, qint64 messageId);
};

template <WRITE_METHOD W> qint64 TgTransport::sendPlainObject(QVariant i)
{
    kgDebug() << "Sending plain object:" << GETID(i.toMap());
    return sendPlainMessage(tlSerialize<W>(i));
}

template <WRITE_METHOD W> qint64 TgTransport::sendMTObject(QVariant i)
{
    kgDebug() << "Sending MT object:" << GETID(i.toMap());
    return sendMTMessage(tlSerialize<W>(i));
}

#endif // TGTRANSPORT_H
