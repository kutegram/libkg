#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include "tgtransport.h"
#include "tgutils.h"
#include <QFile>
#include "crypto.h"
#include <QLinkedList>
#include <QDir>
#include <mbedtls/md5.h>

class TgFileCtx
{
public:
    TgFileCtx(QString fileName);
    TgFileCtx(TgObject input, QString fileName, TgLong fileSize);

    TgLong fileId;
    QFile localFile;
    mbedtls_md5_context digest;
    qint64 length;
    qint64 bytesLeft;
    qint32 fileParts;
    qint32 currentPart;
    bool isBig;
    QLinkedList<qint64> queue;
    TgObject download;
};

//TODO: send message
//TODO: send document
//TODO: send photo
//TODO: record PCM and convert to ogg vorbis

class TgClient : public QObject
{
    Q_OBJECT
private:
    QString clientSessionName;
    TgTransport *_transport;
    QMap<TgLong, TgFileCtx*> processedFiles;
    QMap<TgLong, TgFileCtx*> processedDownloadFiles;
    TgLong currentDownloading;
    QMap<TgLong, TgLong> filePackets;
    QMap<qint32, TgClient*> clientForDc;
    QMap<TgLong, TgInt> migrationForDc;
    bool _main;
    bool _connected;
    bool _initialized;
    bool _authorized;
    TgObject importMethod;
    QDir _cacheDirectory;

public:
    explicit TgClient(QObject *parent = 0, qint32 dcId = 0, QString sessionName = "");
    
    template <WRITE_METHOD W> TgLong sendObject(TgObject i);
    
public slots:
    void start();
    void stop();
    QDir cacheDirectory();

    void resetSession();
    void migrateTo(TgObject config, qint32 dcId);
    TgClient* getClientForDc(qint32 dcId);

    TgInt dcId();
    bool hasSession();
    bool hasUserId();
    TgLong getUserId();
    bool isMain();
    bool isConnected();
    bool isInitialized();
    bool isAuthorized();

    void cancelUpload(TgLong fileId);
    TgLong uploadFile(QString filePath);
    TgLong uploadNextFilePart(TgLong fileId);
    void handleUploadingFile(bool response, TgLong messageId);

    void cancelDownload(TgLong fileId);
    TgLong downloadFile(QString filePath, TgObject inputFile, TgLong fileSize = 0, qint32 dcId = 0, TgLong fileId = 0);
    TgLong downloadNextFilePart();
    TgLong migrateFileTo(TgLong messageId, TgInt dcId);
    void fileProbablyDownloaded(TgLong messageId);
    void handleUploadFile(TgObject response, TgLong messageId);

    TgLong exportAuthorization(qint32 dcId);
    TgLong importAuthorization(qint64 id, QByteArray bytes);

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
    TgLong messagesGetDialogs(qint32 offsetDate = 0, qint32 offsetId = 0, TgObject offsetPeer = emptyPeer(), qint32 limit = 20, qint32 folderId = 0, bool excludePinned = false, qint64 hash = 0);
    TgLong messagesGetDialogsWithOffsets(TgObject offsets = TgObject(), qint32 limit = 20, qint32 folderId = 0, bool excludePinned = false, qint64 hash = 0);
    TgLong authSignUp(QString phoneNumber, QString phoneCodeHash, QString firstName, QString lastName = "");
    TgLong messagesGetHistory(TgObject inputPeer, qint32 offsetId = 0, qint32 offsetDate = 0, qint32 addOffset = 0, qint32 limit = 20, qint32 maxId = 0, qint32 minId = 0, qint64 hash = 0);

signals:
    void connected(bool hasUserId);
    void disconnected(bool hasUserId);
    void initialized(bool hasUserId);
    void rpcError(qint32 errorCode, QString errorMessage, qint64 messageId);
    void authorized(qint64 userId);

    void fileDownloading(qint64 fileId, qint64 downloadedLength, qint64 totalLength);
    void fileDownloaded(qint64 fileId, QString filePath);
    void fileDownloadCanceled(qint64 fileId, QString filePath);

    void fileUploading(qint64 fileId, qint64 uploadedLength, qint64 totalLength);
    void fileUploaded(qint64 fileId, TgObject inputFile);
    void fileUploadCanceled(qint64 fileId);

    void messageChanged(qint64 oldMsg, qint64 newMsg);

    //TODO: rename to objects types
    void helpGetCountriesListResponse(TgObject data, TgLong messageId);
    void authSendCodeResponse(TgObject data, TgLong messageId);
    void authSignInResponse(TgObject data, TgLong messageId);
    void messagesGetDialogsResponse(TgObject data, TgLong messageId);
    void messagesGetHistoryResponse(TgObject data, TgLong messageId);

    void boolResponse(bool response, qint64 messageId);
    void unknownResponse(qint32 conId, QByteArray data, qint64 messageId);
};

template <WRITE_METHOD W> TgLong TgClient::sendObject(TgObject i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
