#include "tgclient.h"

#include <QtEndian>
#include "tlschema.h"
#include <QFileInfo>

TgFileCtx::TgFileCtx(QString fileName)
    : fileId()
    , localFile()
    , md5()
    , length()
    , bytesLeft()
    , fileParts()
    , currentPart()
    , isBig()
    , queue()
{
    fileId = qFromLittleEndian<qint64>((const uchar*) randomBytes(INT64_BYTES).constData());
    localFile.setFileName(fileName);
    MD5_Init(&md5);
    length = localFile.size();
    bytesLeft = length;
    fileParts = (qint32) ((length - 1) / 524288) + 1; //512 KiB
    currentPart = 0;
    isBig = length > 10485760; //10 MB as Telegram says (10 MiB)
}

TgLong TgClient::uploadFile(QString filePath)
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

TgLong TgClient::uploadNextFilePart(TgLong fileId)
{
    //TODO: seek offset and get bytes as part number (offset = partNumber * partSize, length = qMin(524288, length - offset))
    //TODO: send 2-3 parts in queue
    TgFileCtx *ctx = processedFiles.value(fileId, NULL);
    if (ctx == NULL)
        return 0;

    if (ctx->bytesLeft == 0) {
        TGOBJECT(ctx->isBig ? TLType::InputFileBig : TLType::InputFile, inputFile);
        inputFile["id"] = ctx->fileId;
        inputFile["parts"] = ctx->fileParts;
        inputFile["name"] = QFileInfo(ctx->localFile).fileName();

        if (!ctx->isBig) {
            QByteArray md5Hash;
            md5Hash.resize(MD5_DIGEST_LENGTH);
            MD5_Final((unsigned char*) md5Hash.data(), &(ctx->md5));
            inputFile["md5_checksum"] = md5Hash;
        }

        processedFiles.remove(ctx->fileId);
        delete ctx;

        emit fileUploaded(inputFile["id"].toLongLong(), inputFile);

        return 0;
    }

    QByteArray fileBytes = readFully(ctx->localFile, qMin((qint64) 524288, ctx->bytesLeft));
    ctx->bytesLeft -= fileBytes.size();

    if (!ctx->isBig)
        MD5_Update(&(ctx->md5), fileBytes.constData(), fileBytes.size());

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

    emit fileUploading(saveFile["file_id"].toLongLong(), saveFile["file_part"].toInt(), saveFile["file_total_parts"].toInt(), ctx->length);

    return mid;
}

void TgClient::handleUploadFile(bool response, qint64 messageId)
{
    if (!response) {
        kgWarning() << "File upload response is false " << messageId;
        //TODO: resend it?
        return;
    }

    uploadNextFilePart(filePackets.take(messageId));
}
