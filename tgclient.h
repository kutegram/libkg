#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include "tgtransport.h"
#include "tgutils.h"
#include <QFile>
#include "crypto.h"
#include <openssl/md5.h>
#include <QLinkedList>

class TgFileCtx
{
public:
    TgFileCtx(QString fileName);

    TgLong fileId;
    QFile localFile;
    MD5_CTX md5;
    qint64 length;
    qint64 bytesLeft;
    qint32 fileParts;
    qint32 currentPart;
    bool isBig;
    QLinkedList<qint64> queue;
};

class TgClient : public QObject
{
    Q_OBJECT
private:
    TgTransport *_transport;
    QMap<TgLong, TgFileCtx*> processedFiles;
    QMap<TgLong, TgLong> filePackets;

public:
    explicit TgClient(QObject *parent = 0, QString sessionName = "");
    
    template <WRITE_METHOD W> TgLong sendObject(TgObject i);
    
public slots:
    void start();
    void stop();

    void resetSession();

    bool hasSession();
    bool hasUserId();
    TgLong getUserId();

    TgLong uploadFile(QString filePath);
    TgLong uploadNextFilePart(TgLong fileId);
    void handleUploadFile(bool response, qint64 messageId);

    void handleObject(QByteArray data, qint64 messageId);
    void handleBool(bool response, qint64 messageId);

    void handleConnected();
    void handleDisconnected();
    void handleInitialized();
    void handleRpcError(qint32 errorCode, QString errorMessage, qint64 messageId);
    void handleAuthorized(qint64 userId);
    void handleMessageChanged(qint64 oldMsg, qint64 newMsg);

    TgLong helpGetCountriesList(qint32 hash = 0, QString langCode = "");
    TgLong authSendCode(QString phoneNumber);
    TgLong authSignIn(QString phoneNumber, QString phoneCodeHash, QString phoneCode);
    TgLong messagesGetDialogs(qint32 offsetDate = 0, qint32 offsetId = 0, TgObject offsetPeer = emptyPeer(), qint32 limit = 1000000, qint32 folderId = 0, bool excludePinned = false, qint64 hash = 0);
    TgLong messagesGetDialogsWithOffsets(TgObject offsets = TgObject(), qint32 limit = 1000000, qint32 folderId = 0, bool excludePinned = false, qint64 hash = 0);
    TgLong authSignUp(QString phoneNumber, QString phoneCodeHash, QString firstName, QString lastName = "");

signals:
    void connected(bool hasUserId);
    void disconnected(bool hasUserId);
    void initialized(bool hasUserId);
    void rpcError(qint32 errorCode, QString errorMessage, qint64 messageId);
    void authorized(qint64 userId);

    void fileUploading(qint64 fileId, qint32 filePart, qint32 totalParts, qint64 totalLength);
    void fileUploaded(qint64 fileId, TgObject inputFile);

    void messageChanged(qint64 oldMsg, qint64 newMsg);

    void helpGetCountriesListResponse(TgObject object, TgLong messageId);
    void authSendCodeResponse(TgObject object, TgLong messageId);
    void authSignInResponse(TgObject object, TgLong messageId);
    void messagesGetDialogsResponse(TgObject object, TgLong messageId);

    void boolResponse(bool response, qint64 messageId);
    void unknownResponse(qint32 conId, QByteArray object, qint64 messageId);
};

template <WRITE_METHOD W> TgLong TgClient::sendObject(TgObject i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
