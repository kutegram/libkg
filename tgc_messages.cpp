#include "tgclient.h"

#include "tlschema.h"

TgLong TgClient::messagesGetDialogs(qint32 offsetDate, qint32 offsetId, TgObject offsetPeer, qint32 limit, qint32 folderId, bool excludePinned, qint64 hash)
{
    TGOBJECT(TLType::MessagesGetDialogsMethod, method);

    method["exclude_pinned"] = excludePinned;
    method["folder_id"] = folderId == 1 ? 1 : 0;
    method["offset_date"] = offsetDate;
    method["offset_id"] = offsetId;
    method["offset_peer"] = GETID(offsetPeer) == 0 ? emptyPeer() : offsetPeer;
    method["limit"] = limit;
    method["hash"] = hash;

    return sendObject<&writeTLMethodMessagesGetDialogs>(method);
}

TgLong TgClient::messagesGetDialogsWithOffsets(TgObject offsets, qint32 limit, qint32 folderId, bool excludePinned, qint64 hash)
{
    return messagesGetDialogs(offsets["offset_date"].toInt(), offsets["offset_id"].toInt(), offsets["offset_peer"].toMap(), limit, folderId, excludePinned, hash);
}
