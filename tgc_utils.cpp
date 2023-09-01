#include "tgclient.h"

#include "tlschema.h"

using namespace TLType;

TgObject TgClient::emptyInputPeer()
{
    TGOBJECT(InputPeerEmpty, peer);
    return peer;
}

TgObject TgClient::selfInputPeer()
{
    TGOBJECT(InputPeerSelf, peer);
    return peer;
}

TgObject TgClient::toInputPeer(TgObject obj)
{
    switch (ID(obj)) {
    case Chat:
    case ChatForbidden:
    case PeerChat:
    {
        TGOBJECT(InputPeerChat, v);

        v["chat_id"] = obj["id"];

        return v;
    }
    case Channel:
    case ChannelForbidden:
    {
        TGOBJECT(InputPeerChannel, v);

        v["channel_id"] = obj["id"];
        v["access_hash"] = obj["access_hash"];

        return v;
    }
    case TLType::User:
    {
        TGOBJECT(InputPeerUser, v);

        v["user_id"] = obj["id"];
        v["access_hash"] = obj["access_hash"];

        return v;
    }
    case InputPeerEmpty:
    case InputPeerSelf:
    case InputPeerChat:
    case InputPeerUser:
    case InputPeerChannel:
    case InputPeerUserFromMessage:
    case InputPeerChannelFromMessage:
        return obj;
    default:
    {
        TGOBJECT(InputPeerEmpty, v);

        return v;
    }
    }
}

TgVariant getPeerIdVariant(TgObject obj)
{
    switch (ID(obj)) {
    case PeerUser:
        return obj["user_id"];
    case PeerChat:
        return obj["chat_id"];
    case PeerChannel:
        return obj["channel_id"];
    case InputUserEmpty:
        return QVariant();
    case InputUserSelf:
        return QVariant(); //TODO: current account ID
    case InputUser:
        return obj["user_id"];
    case InputUserFromMessage:
        return obj["user_id"];
    case InputPeerEmpty:
        return QVariant();
    case InputPeerSelf:
        return QVariant(); //TODO: current account ID
    case InputPeerChat:
        return obj["chat_id"];
    case InputPeerUser:
        return obj["user_id"];
    case InputPeerChannel:
        return obj["channel_id"];
    case InputPeerUserFromMessage:
        return obj["user_id"];
    case InputPeerChannelFromMessage:
        return obj["channel_id"];
    case TLType::User:
        return obj["id"];
    case UserEmpty:
        return obj["id"];
    case Chat:
        return obj["id"];
    case ChatEmpty:
        return obj["id"];
    case ChatForbidden:
        return obj["id"];
    case Channel:
        return obj["id"];
    case ChannelForbidden:
        return obj["id"];
    default:
        return QVariant();
    }
}

TgLongVariant TgClient::getPeerId(TgObject obj)
{
    return getPeerIdVariant(obj).toLongLong();
}

bool TgClient::isChat(TgObject obj)
{
    return commonPeerType(obj) == TLType::Chat;
}

bool TgClient::isUser(TgObject obj)
{
    return commonPeerType(obj) == TLType::User;
}

bool TgClient::isChannel(TgObject obj)
{
    return obj["broadcast"].toBool();
}

TgLongVariant TgClient::commonPeerType(TgObject obj)
{
    switch (ID(obj)) {
    case InputUserEmpty:
    case InputUserSelf:
    case InputUser:
    case InputUserFromMessage:
    case TLType::User:
    case UserEmpty:
    case PeerUser:
    case InputPeerEmpty:
    case InputPeerSelf:
    case InputPeerUser:
    case InputPeerUserFromMessage:
        return TLType::User;
    case Chat:
    case ChatEmpty:
    case ChatForbidden:
    case Channel:
    case ChannelForbidden:
    case PeerChat:
    case PeerChannel:
    case InputPeerChat:
    case InputPeerChannel:
    case InputPeerChannelFromMessage:
        return TLType::Chat;
    default:
        return 0;
    }
}

bool TgClient::peersEqual(TgObject peer1, TgObject peer2)
{
    return commonPeerType(peer1) == commonPeerType(peer2) && getPeerId(peer1) == getPeerId(peer2);
}

TgObject TgClient::getDialogsOffsets(TgObject dialogs)
{
    TgList dialogsList = dialogs["dialogs"].toList();
    TgList messagesList = dialogs["messages"].toList();
    TgList usersList = dialogs["users"].toList();
    TgList chatsList = dialogs["chats"].toList();

    TgObject lastDialog = dialogsList.isEmpty() ? TgObject() : dialogsList.last().toMap();
    TgObject lastPeerDialog = lastDialog["peer"].toMap();
    TgInt lastTopMessage = lastDialog["top_message"].toInt();

    TgObject lastMessage;
    TgObject lastPeer;

    for (qint32 i = 0; i < messagesList.size(); ++i) {
        TgObject message = messagesList[i].toMap();
        if (peersEqual(message["peer_id"].toMap(), lastPeerDialog)
                && message["id"].toInt() == lastTopMessage) {
            lastMessage = message;
            break;
        }
    }

    if (isUser(lastPeerDialog)) {
        for (qint32 i = 0; i < usersList.size(); ++i) {
            if (getPeerId(usersList[i].toMap()) == getPeerId(lastPeerDialog)) {
                lastPeer = usersList[i].toMap();
                break;
            }
        }
    }
    else {
        for (qint32 i = 0; i < chatsList.size(); ++i) {
            if (getPeerId(chatsList[i].toMap()) == getPeerId(lastPeerDialog)) {
                lastPeer = chatsList[i].toMap();
                break;
            }
        }
    }

    TgObject offsets;
    offsets["offset_date"] = lastMessage["date"].toInt();
    offsets["offset_id"] = lastTopMessage;
    offsets["offset_peer"] = toInputPeer(lastPeer);

    return offsets;
}
