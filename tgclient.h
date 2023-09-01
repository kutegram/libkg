#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include "tgtransport.h"
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

//TODO: research behavior of upload / download with disconnecting
//TODO: record PCM and convert to ogg vorbis
//TODO: mutexes for migrations / file operations

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
    static qint32 getObjectId(TgObject var);
    static QByteArray getJpegPayload(QByteArray bytes);

    void start();
    void stop();
    QDir cacheDirectory();

    void resetSession();
    void migrateTo(TgObject config, qint32 dcId);
    TgClient* getClientForDc(qint32 dcId);

    TgInt dcId();
    bool hasSession();
    bool hasUserId();
    TgLongVariant getUserId();
    bool isMain();
    bool isConnected();
    bool isInitialized();
    bool isAuthorized();

    void cancelUpload(TgLongVariant fileId);
    TgLongVariant uploadFile(QString filePath);
    TgLongVariant uploadNextFilePart(TgLongVariant fileId);
    void handleUploadingFile(bool response, TgLongVariant messageId);

    void cancelDownload(TgLongVariant fileId);
    TgLongVariant downloadFile(QString filePath, TgObject inputFile, TgLongVariant fileSize = 0, TgInt dcId = 0, TgLongVariant fileId = 0);
    TgLongVariant downloadNextFilePart();
    TgLongVariant migrateFileTo(TgLongVariant messageId, TgInt dcId);
    void fileProbablyDownloaded(TgLongVariant messageId);
    void handleDownloadingFile(TgObject response, TgLongVariant messageId);

    qint64 exportAuthorization(qint32 dcId);
    qint64 importAuthorization(qint64 id, QByteArray bytes);

    void handleObject(QByteArray data, qint64 messageId);
    void handleBool(bool response, qint64 messageId);

    void handleMessageChanged(qint64 oldMsg, qint64 newMsg);

    void handleConnected();
    void handleDisconnected();
    void handleInitialized();
    void handleRpcError(qint32 errorCode, QString errorMessage, qint64 messageId);
    void handleAuthorized(qint64 userId);
    void handleTFARequired();

    static TgObject emptyInputPeer();
    static TgObject selfInputPeer();
    static TgObject toInputPeer(TgObject obj);
    static TgLongVariant getPeerId(TgObject obj);
    static bool isUser(TgObject obj);
    static bool isChat(TgObject obj);
    static bool isChannel(TgObject obj);
    static TgObject getDialogsOffsets(TgObject dialogs);
    static TgLongVariant commonPeerType(TgObject obj);
    static bool peersEqual(TgObject peer1, TgObject peer2);

    TgLongVariant helpGetCountriesList(qint32 hash = 0, QString langCode = "");
    TgLongVariant authSendCode(QString phoneNumber);
    TgLongVariant authSignIn(QString phoneNumber, QString phoneCodeHash, QString phoneCode);
    TgLongVariant messagesGetDialogs(qint32 offsetDate = 0, qint32 offsetId = 0, TgObject offsetPeer = TgObject(), qint32 limit = 20, qint32 folderId = 0, bool excludePinned = false, TgLongVariant hash = 0);
    TgLongVariant messagesGetDialogsWithOffsets(TgObject offsets = TgObject(), qint32 limit = 20, qint32 folderId = 0, bool excludePinned = false, TgLongVariant hash = 0);
    TgLongVariant authSignUp(QString phoneNumber, QString phoneCodeHash, QString firstName, QString lastName = "");
    TgLongVariant messagesGetHistory(TgObject inputPeer, qint32 offsetId = 0, qint32 offsetDate = 0, qint32 addOffset = 0, qint32 limit = 20, qint32 maxId = 0, qint32 minId = 0, TgLongVariant hash = 0);
    TgLongVariant messagesSendMessage(TgObject inputPeer, QString message, TgObject media = TgObject(), TgLongVariant randomId = randomLong());
    TgLongVariant messagesSendMedia(TgObject inputPeer, TgObject media, QString message = "", TgLongVariant randomId = randomLong());

    static void registerQML();

signals:
    void connected(bool hasUserId);
    void disconnected(bool hasUserId);
    void initialized(bool hasUserId);
    void rpcError(qint32 errorCode, QString errorMessage, TgLongVariant messageId);
    void authorized(TgLongVariant userId);
    void tfaRequired();

    void fileDownloading(TgLongVariant fileId, TgLongVariant processedLength, TgLongVariant totalLength, qint32 progressPercentage);
    void fileDownloaded(TgLongVariant fileId, QString filePath);
    void fileDownloadCanceled(TgLongVariant fileId, QString filePath);

    void fileUploading(TgLongVariant fileId, TgLongVariant processedLength, TgLongVariant totalLength, qint32 progressPercentage);
    void fileUploaded(TgLongVariant fileId, TgObject inputFile);
    void fileUploadCanceled(TgLongVariant fileId);

    void messageChanged(TgLongVariant oldMsg, TgLongVariant newMsg);

    //TODO: rename to objects types
    void helpGetCountriesListResponse(TgObject data, TgLongVariant messageId);
    void authSendCodeResponse(TgObject data, TgLongVariant messageId);
    void authSignInResponse(TgObject data, TgLongVariant messageId);
    void messagesGetDialogsResponse(TgObject data, TgLongVariant messageId);
    void messagesGetHistoryResponse(TgObject data, TgLongVariant messageId);

    void boolResponse(bool response, TgLongVariant messageId);
    void unknownResponse(qint32 conId, QByteArray data, TgLongVariant messageId);
};

template <WRITE_METHOD W> TgLong TgClient::sendObject(TgObject i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
