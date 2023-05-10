#include "tgutils.h"

#include "tlschema.h"

using namespace TLType;

TgObject emptyPeer()
{
    TGOBJECT(InputPeerEmpty, peer);
    return peer;
}

TgObject selfPeer()
{
    TGOBJECT(InputPeerSelf, peer);
    return peer;
}

TgObject toInputPeer(TgObject obj)
{
    switch (ID(obj)) {
    case Chat:
    case ChatForbidden:
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

TgLong getPeerId(TgObject obj)
{
    return getPeerIdVariant(obj).toLongLong();
}

bool isChat(TgObject obj)
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
        return false;
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
        return true;
    default:
        return false;
    }
}

bool isUser(TgObject obj)
{
    return !isChat(obj);
}

TgObject getDialogsOffsets(TgObject dialogs)
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
        if (getPeerId(message["peer_id"].toMap()) == getPeerId(lastPeerDialog)
                && message["id"].toInt() == lastTopMessage) {
            lastMessage = message;
            break;
        }
    }

    if (isUser(lastDialog["peer"].toMap())) {
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

QByteArray getJpegPayload(QByteArray bytes)
{
    if (bytes.size() < 3 || bytes[0] != '\x01') {
        return QByteArray();
    }
    const char header[] = "\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49"
                    "\x46\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00\xff\xdb\x00\x43\x00\x28\x1c"
                    "\x1e\x23\x1e\x19\x28\x23\x21\x23\x2d\x2b\x28\x30\x3c\x64\x41\x3c\x37\x37"
                    "\x3c\x7b\x58\x5d\x49\x64\x91\x80\x99\x96\x8f\x80\x8c\x8a\xa0\xb4\xe6\xc3"
                    "\xa0\xaa\xda\xad\x8a\x8c\xc8\xff\xcb\xda\xee\xf5\xff\xff\xff\x9b\xc1\xff"
                    "\xff\xff\xfa\xff\xe6\xfd\xff\xf8\xff\xdb\x00\x43\x01\x2b\x2d\x2d\x3c\x35"
                    "\x3c\x76\x41\x41\x76\xf8\xa5\x8c\xa5\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
                    "\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
                    "\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
                    "\xf8\xf8\xf8\xf8\xf8\xff\xc0\x00\x11\x08\x00\x00\x00\x00\x03\x01\x22\x00"
                    "\x02\x11\x01\x03\x11\x01\xff\xc4\x00\x1f\x00\x00\x01\x05\x01\x01\x01\x01"
                    "\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"
                    "\x09\x0a\x0b\xff\xc4\x00\xb5\x10\x00\x02\x01\x03\x03\x02\x04\x03\x05\x05"
                    "\x04\x04\x00\x00\x01\x7d\x01\x02\x03\x00\x04\x11\x05\x12\x21\x31\x41\x06"
                    "\x13\x51\x61\x07\x22\x71\x14\x32\x81\x91\xa1\x08\x23\x42\xb1\xc1\x15\x52"
                    "\xd1\xf0\x24\x33\x62\x72\x82\x09\x0a\x16\x17\x18\x19\x1a\x25\x26\x27\x28"
                    "\x29\x2a\x34\x35\x36\x37\x38\x39\x3a\x43\x44\x45\x46\x47\x48\x49\x4a\x53"
                    "\x54\x55\x56\x57\x58\x59\x5a\x63\x64\x65\x66\x67\x68\x69\x6a\x73\x74\x75"
                    "\x76\x77\x78\x79\x7a\x83\x84\x85\x86\x87\x88\x89\x8a\x92\x93\x94\x95\x96"
                    "\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3\xb4\xb5\xb6"
                    "\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4\xd5\xd6"
                    "\xd7\xd8\xd9\xda\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf1\xf2\xf3\xf4"
                    "\xf5\xf6\xf7\xf8\xf9\xfa\xff\xc4\x00\x1f\x01\x00\x03\x01\x01\x01\x01\x01"
                    "\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"
                    "\x09\x0a\x0b\xff\xc4\x00\xb5\x11\x00\x02\x01\x02\x04\x04\x03\x04\x07\x05"
                    "\x04\x04\x00\x01\x02\x77\x00\x01\x02\x03\x11\x04\x05\x21\x31\x06\x12\x41"
                    "\x51\x07\x61\x71\x13\x22\x32\x81\x08\x14\x42\x91\xa1\xb1\xc1\x09\x23\x33"
                    "\x52\xf0\x15\x62\x72\xd1\x0a\x16\x24\x34\xe1\x25\xf1\x17\x18\x19\x1a\x26"
                    "\x27\x28\x29\x2a\x35\x36\x37\x38\x39\x3a\x43\x44\x45\x46\x47\x48\x49\x4a"
                    "\x53\x54\x55\x56\x57\x58\x59\x5a\x63\x64\x65\x66\x67\x68\x69\x6a\x73\x74"
                    "\x75\x76\x77\x78\x79\x7a\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x92\x93\x94"
                    "\x95\x96\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3\xb4"
                    "\xb5\xb6\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4"
                    "\xd5\xd6\xd7\xd8\xd9\xda\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf2\xf3\xf4"
                    "\xf5\xf6\xf7\xf8\xf9\xfa\xff\xda\x00\x0c\x03\x01\x00\x02\x11\x03\x11\x00"
                    "\x3f\x00";
    const char footer[] = "\xff\xd9";
    QByteArray real(header, sizeof(header) - 1);
    real[164] = bytes[1];
    real[166] = bytes[2];
    return real + bytes.mid(3) + QByteArray::fromRawData(footer, sizeof(footer) - 1);
}
