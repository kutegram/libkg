#include "tgclient.h"

#include "tlschema.h"

TgLongVariant TgClient::messagesGetDialogs(qint32 offsetDate, qint32 offsetId, TgObject offsetPeer, qint32 limit, qint32 folderId, bool excludePinned, TgLongVariant hash)
{
    TGOBJECT(TLType::MessagesGetDialogsMethod, method);

    if (excludePinned) method["exclude_pinned"] = true;
    method["folder_id"] = folderId == 1 ? 1 : 0;
    method["offset_date"] = offsetDate;
    method["offset_id"] = offsetId;
    method["offset_peer"] = GETID(offsetPeer) == 0 ? emptyPeer() : offsetPeer;
    method["limit"] = limit;
    method["hash"] = hash;

    return sendObject<&writeTLMethodMessagesGetDialogs>(method);
}

TgLongVariant TgClient::messagesGetDialogsWithOffsets(TgObject offsets, qint32 limit, qint32 folderId, bool excludePinned, TgLongVariant hash)
{
    return messagesGetDialogs(offsets["offset_date"].toInt(), offsets["offset_id"].toInt(), offsets["offset_peer"].toMap(), limit, folderId, excludePinned, hash);
}

TgLongVariant TgClient::messagesGetHistory(TgObject inputPeer, qint32 offsetId, qint32 offsetDate, qint32 addOffset, qint32 limit, qint32 maxId, qint32 minId, TgLongVariant hash)
{
    TGOBJECT(TLType::MessagesGetHistoryMethod, method);

    method["peer"] = toInputPeer(inputPeer);
    method["offset_id"] = offsetId;
    method["offset_date"] = offsetDate;
    method["add_offset"] = addOffset;
    method["limit"] = limit;
    method["max_id"] = maxId;
    method["min_id"] = minId;
    method["hash"] = hash;

    return sendObject<&writeTLMethodMessagesGetHistory>(method);
}
