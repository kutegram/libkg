#include "tgclient.h"

#include <QtEndian>
#include <QFileInfo>
#include "tlschema.h"

#define FILE_PART_SIZE 524288

TgFileCtx::TgFileCtx(QString fileName)
    : fileId()
    , localFile()
    , length()
    , bytesLeft()
    , fileParts()
    , currentPart()
    , isBig()
    , queue()
    , download()
{
    fileId = randomLong();
    localFile.setFileName(fileName);
    length = localFile.size();
    bytesLeft = length;
    fileParts = (qint32) ((length - 1) / FILE_PART_SIZE) + 1; //512 KiB
    currentPart = 0;
    isBig = length > 10485760; //10 MB as Telegram says (10 MiB)
}

TgFileCtx::TgFileCtx(TgObject input, QString fileName, TgLong fileSize)
    : fileId()
    , localFile()
    , length()
    , bytesLeft()
    , fileParts()
    , currentPart()
    , isBig()
    , queue()
    , download()
{
    fileId = randomLong();
    localFile.setFileName(fileName);
    length = fileSize;
    bytesLeft = length;
    fileParts = (qint32) ((length - 1) / FILE_PART_SIZE) + 1; //512 KiB
    currentPart = 0;
    isBig = length > 10485760; //10 MB as Telegram says (10 MiB)
    download = input;
}

TgLongVariant TgClient::downloadFile(QString filePath, TgObject inputFile, TgLongVariant fileSize, qint32 dcId, TgLongVariant fileId)
{
    //TODO: file references
    //TODO: CDN DCs

    if ((isUser(inputFile) || isChat(inputFile)) && GETID(inputFile["photo"].toMap()) != 0) {
        TGOBJECT(TLType::InputPeerPhotoFileLocation, fileLocation);
        fileLocation["peer"] = toInputPeer(inputFile);
        fileLocation["photo_id"] = inputFile["photo"].toMap()["photo_id"];

        return downloadFile(filePath, fileLocation, fileSize, inputFile["photo"].toMap()["dc_id"].toInt(), fileId);
    }

    switch (GETID(inputFile)) {
    case TLType::MessageMediaPhoto:
        return downloadFile(filePath, inputFile["photo"].toMap(), fileSize, dcId, fileId);
    case TLType::MessageMediaDocument:
        return downloadFile(filePath, inputFile["document"].toMap(), fileSize, dcId, fileId);
    case TLType::Photo:
    {
        TGOBJECT(TLType::InputPhotoFileLocation, loc);
        loc["id"] = inputFile["id"];
        loc["access_hash"] = inputFile["access_hash"];
        loc["file_reference"] = inputFile["file_reference"];
        TgList sizes = inputFile["sizes"].toList();
        TgObject thumbSize;

        for (qint32 i = 0; i < sizes.size(); ++i) {
            TgObject obj = sizes[i].toMap();
            if (obj["w"].toInt() > thumbSize["w"].toInt() || obj["h"].toInt() > thumbSize["h"].toInt())
                thumbSize = obj;
        }

        loc["thumb_size"] = thumbSize["type"];
        return downloadFile(filePath, loc, thumbSize["size"].toLongLong(), inputFile["dc_id"].toInt(), fileId);
    }
    case TLType::Document:
    {
        TGOBJECT(TLType::InputDocumentFileLocation, loc);
        loc["id"] = inputFile["id"];
        loc["access_hash"] = inputFile["access_hash"];
        loc["file_reference"] = inputFile["file_reference"];
        loc["thumb_size"] = "";
        return downloadFile(filePath, loc, inputFile["size"].toLongLong(), inputFile["dc_id"].toInt(), fileId);
    }
    }

    if (inputFile["id"].toLongLong() == 0 && inputFile["photo_id"].toLongLong() == 0) {
        return 0;
    }

    TgFileCtx *ctx = new TgFileCtx(inputFile, filePath, fileSize.toLongLong());

    if (!ctx->localFile.open(QFile::WriteOnly)) {
        delete ctx;
        return 0;
    }

    if (fileId != 0) {
        ctx->fileId = fileId.toLongLong();
    }

    TgClient* dcClient = getClientForDc(dcId);
    if (dcClient == 0) {
        delete ctx;
        return 0;
    }

    dcClient->processedDownloadFiles.insert(ctx->fileId, ctx);
    dcClient->downloadNextFilePart();

    //TODO: load files one after other sequensively (only one file at time)

    return ctx->fileId;
}

TgLongVariant TgClient::migrateFileTo(TgLongVariant messageId, TgInt dcId)
{
    TgLong fileId = filePackets.take(messageId.toLongLong());
    TgFileCtx *ctx = processedDownloadFiles.take(fileId);
    if (ctx == NULL)
        return 0;

    TgClient* client = isMain() ? this : static_cast<TgClient*>(parent());
    if (client == 0) {
        client = this;
    }
    ctx->localFile.close();
    TgLong mid = client->downloadFile(ctx->localFile.fileName(), ctx->download, ctx->length, dcId, ctx->fileId).toLongLong();
    delete ctx;

    currentDownloading = 0;
    downloadNextFilePart();
    return mid;
}

void TgClient::fileProbablyDownloaded(TgLongVariant messageId)
{
    TgLong fileId = filePackets.take(messageId.toLongLong());
    TgFileCtx *ctx = processedDownloadFiles.take(fileId);
    if (ctx == NULL)
        return;

    TgClient* client = isMain() ? this : static_cast<TgClient*>(parent());
    if (client == 0) {
        client = this;
    }
    ctx->localFile.close();
    emit client->fileDownloaded(ctx->fileId, ctx->localFile.fileName());
    delete ctx;

    currentDownloading = 0;
    downloadNextFilePart();
}

void TgClient::cancelDownload(TgLongVariant fileId)
{
    TgFileCtx *ctx = processedDownloadFiles.take(fileId.toLongLong());
    if (ctx == NULL)
        return;

    TgClient* client = isMain() ? this : static_cast<TgClient*>(parent());
    if (client == 0) {
        client = this;
    }
    ctx->localFile.close();
    ctx->localFile.remove();
    emit client->fileDownloadCanceled(fileId, ctx->localFile.fileName());
    delete ctx;

    currentDownloading = 0;
    downloadNextFilePart();
}

TgLongVariant TgClient::downloadNextFilePart()
{
    //TODO: seek offset and get bytes as part number (offset = partNumber * partSize, length = qMin(FILE_PART_SIZE, length - offset))
    //TODO: get 2-3 parts in queue
    //TODO: Verify hash chunks
    if (!isAuthorized()) {
        return 0;
    }

    if (processedDownloadFiles.isEmpty()) {
        currentDownloading = 0;
        return 0;
    }

    if (currentDownloading == 0) {
        currentDownloading = processedDownloadFiles.begin().key();
    }

    TgFileCtx *ctx = processedDownloadFiles.value(currentDownloading, NULL);
    if (ctx == NULL)
        return 0;

    if (!ctx->queue.isEmpty()) {
        return 0;
    }

    if (ctx->length != 0 && ctx->bytesLeft <= 0) {
        processedDownloadFiles.remove(ctx->fileId);

        TgClient* client = isMain() ? this : static_cast<TgClient*>(parent());
        if (client == 0) {
            client = this;
        }
        ctx->localFile.close();
        emit client->fileDownloaded(ctx->fileId, ctx->localFile.fileName());
        delete ctx;

        return 0;
    }

    TGOBJECT(TLType::UploadGetFileMethod, getFile);
    getFile["location"] = ctx->download;
    getFile["offset"] = (ctx->currentPart++) * (qint64) FILE_PART_SIZE;
    getFile["limit"] = FILE_PART_SIZE;

    TgLong mid = sendObject<&writeTLMethodUploadGetFile>(getFile);

    ctx->queue.append(mid);
    filePackets.insert(mid, ctx->fileId);

    return mid;
}

void TgClient::handleDownloadingFile(TgObject response, TgLongVariant messageId)
{
    TgLong fileId = filePackets.take(messageId.toLongLong());
    TgFileCtx *ctx = processedDownloadFiles.value(fileId, NULL);
    if (ctx != NULL) {
        QByteArray bytes = response["bytes"].toByteArray();
        ctx->localFile.write(bytes);
        ctx->bytesLeft -= bytes.size();
        ctx->queue.removeOne(messageId.toLongLong());

        TgClient* client = isMain() ? this : static_cast<TgClient*>(parent());
        if (client == 0) {
            client = this;
        }

        if (bytes.size() != FILE_PART_SIZE) {
            processedDownloadFiles.remove(fileId);
            ctx->localFile.close();
            emit client->fileDownloaded(ctx->fileId, ctx->localFile.fileName());
            delete ctx;

            currentDownloading = 0;
            downloadNextFilePart();
            return;
        }
        else  {
            qint64 processed = ctx->length - ctx->bytesLeft;
            emit client->fileDownloading(ctx->fileId, processed, ctx->length, (qint32) (processed * 100 / ctx->length));
        }
    }

    downloadNextFilePart();
}

TgLongVariant TgClient::uploadFile(QString filePath)
{
    TgFileCtx *ctx = new TgFileCtx(filePath);

    if (ctx->length == 0) {
        delete ctx;
        return 0;
    }

    if (!ctx->localFile.open(QFile::ReadOnly)) {
        delete ctx;
        return 0;
    }

    processedFiles.insert(ctx->fileId, ctx);
    uploadNextFilePart(ctx->fileId);

    return ctx->fileId;
}

TgLongVariant TgClient::uploadNextFilePart(TgLongVariant fileId)
{
    //TODO: seek offset and get bytes as part number (offset = partNumber * partSize, length = qMin(FILE_PART_SIZE, length - offset))
    //TODO: send 2-3 parts in queue
    //TODO: FILE_PARTS_INVALID
    //TODO: FILE_PART_X_MISSING
    //TODO: MD5_CHECKSUM_INVALID

    if (!isAuthorized()) {
        return 0;
    }

    TgFileCtx *ctx = processedFiles.value(fileId.toLongLong(), NULL);
    if (ctx == NULL)
        return 0;

    if (ctx->length != 0 && ctx->bytesLeft <= 0) {
        TGOBJECT(ctx->isBig ? TLType::InputFileBig : TLType::InputFile, inputFile);
        inputFile["id"] = ctx->fileId;
        inputFile["parts"] = ctx->fileParts;
        inputFile["name"] = QFileInfo(ctx->localFile).fileName();

        processedFiles.remove(ctx->fileId);
        delete ctx;

        emit fileUploaded(inputFile["id"].toLongLong(), inputFile);

        return 0;
    }

    QByteArray fileBytes = readFully(ctx->localFile, qMin((qint64) FILE_PART_SIZE, ctx->bytesLeft));
    ctx->bytesLeft -= fileBytes.size();

    TGOBJECT(ctx->isBig ? TLType::UploadSaveBigFilePartMethod : TLType::UploadSaveFilePartMethod, saveFile);
    saveFile["file_id"] = ctx->fileId;
    saveFile["file_total_parts"] = ctx->fileParts;
    saveFile["file_part"] = (ctx->currentPart)++;
    saveFile["bytes"] = fileBytes;

    TgLong mid;
    if (ctx->isBig) {
        mid = sendObject<&writeTLMethodUploadSaveBigFilePart>(saveFile);
    }
    else {
        mid = sendObject<&writeTLMethodUploadSaveFilePart>(saveFile);
    }

    ctx->queue << mid;
    filePackets.insert(mid, ctx->fileId);

    qint64 processed = qMin(ctx->length, saveFile["file_part"].toInt() * (qint64) FILE_PART_SIZE);
    emit fileUploading(saveFile["file_id"].toLongLong(), processed, ctx->length, (qint32) (processed * 100 / ctx->length));

    return mid;
}

void TgClient::cancelUpload(TgLongVariant fileId)
{
    TgFileCtx *ctx = processedFiles.take(fileId.toLongLong());
    if (ctx == NULL)
        return;

    ctx->localFile.close();
    emit fileUploadCanceled(fileId);
    delete ctx;
}

void TgClient::handleUploadingFile(bool response, TgLongVariant messageId)
{
    TgLong fileId = filePackets.take(messageId.toLongLong());

    if (!response) {
        kgWarning() << "File upload response is false " << messageId;
        cancelUpload(fileId);
        return;
    }

    uploadNextFilePart(fileId);
}
