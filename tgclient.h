#ifndef TGCLIENT_H
#define TGCLIENT_H

#include <QObject>
#include "tgtransport.h"
#include <QFile>
#include "crypto.h"
#include <QList>
#include <QDir>

class TgFileCtx
{
public:
    TgFileCtx(QString fileName);
    TgFileCtx(TgObject input, QString fileName, TgLong fileSize);

    TgLong fileId;
    QFile localFile;
    qint64 length;
    qint64 bytesLeft;
    qint32 fileParts;
    qint32 currentPart;
    bool isBig;
    QList<qint64> queue;
    TgObject download;
};

//TODO: research behavior of upload / download with disconnecting and session change
//TODO: record PCM and convert to ogg vorbis
//TODO: mutexes for migrations / file operations

class TgClient : public QObject
{
    Q_OBJECT
private:
    QString clientSessionName;
    TgTransport *_transport;
    QHash<TgLong, TgFileCtx*> processedFiles;
    QHash<TgLong, TgFileCtx*> processedDownloadFiles;
    TgLong currentDownloading;
    QHash<TgLong, TgLong> filePackets;
    QHash<qint32, TgClient*> clientForDc;
    QHash<TgLong, TgInt> migrationForDc;
    bool _main;
    bool _connected;
    bool _initialized;
    bool _authorized;
    TgObject importMethod;
    QDir _cacheDirectory;
    QDir _sessionDirectory;

public:
    explicit TgClient(QObject *parent = 0, qint32 dcId = 0, QString sessionName = "");
    
    template <WRITE_METHOD W> TgLong sendObject(TgObject i);
    
public slots:
    static qint32 getObjectId(TgObject var);
    static QByteArray getJpegPayload(QByteArray bytes);

    void start();
    void stop();

    QDir cacheDirectory();
    QDir sessionDirectory();
    QString sessionName();
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
    void handleVector(QByteArray data, qint64 messageId);

    void handleBool(bool response, qint64 messageId);

    void handleMessageChanged(qint64 oldMsg, qint64 newMsg);

    void handleConnected();
    void handleSocketError(qint32 errorCode, QString errorMessage);
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
    static bool isGroup(TgObject obj);
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
    TgLongVariant messagesGetDialogFilters();
    TgLongVariant usersGetUsers(TgVector users = TgList());

    static void registerQML();

signals:
    void connected(bool hasUserId);
    void socketError(qint32 errorCode, QString errorMessage);
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

    void helpCountriesListResponse(TgObject data, TgLongVariant messageId);
    void authSentCodeResponse(TgObject data, TgLongVariant messageId);
    //emits for auth.Authorizations that are visible to end user only
    void authAuthorizationResponse(TgObject data, TgLongVariant messageId);
    void messagesDialogsResponse(TgObject data, TgLongVariant messageId);
    void messagesMessagesResponse(TgObject data, TgLongVariant messageId);

    void vectorDialogFilterResponse(TgVector data, TgLongVariant messageId);
    void vectorUserResponse(TgVector data, TgLongVariant messageId);

    void boolResponse(bool response, TgLongVariant messageId);
    void unknownResponse(qint32 conId, QByteArray data, TgLongVariant messageId);

    void gotUpdate(TgObject update, TgLongVariant messageId, TgList users, TgList chats, qint32 date, qint32 seq, qint32 seqStart);
    void gotMessageUpdate(TgObject messageUpdate, TgLongVariant messageId);
};

template <WRITE_METHOD W> TgLong TgClient::sendObject(TgObject i)
{
    return _transport->sendMTObject<W>(i);
}

#endif // TGCLIENT_H
