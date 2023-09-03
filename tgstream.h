#ifndef TELEGRAMSTREAM_H
#define TELEGRAMSTREAM_H

#include <QObject>
#include <QDataStream>
#include <QVariantMap>
#include <QByteArray>
#include <QVariantMap>
#include <QVariantList>
#include <QByteArray>

//TODO: inline methods

class TelegramStream : public QObject
{
    Q_OBJECT
private:
    QByteArray array;
public:
    QDataStream stream;
    explicit TelegramStream(QByteArray input = QByteArray());

    void skipRawBytes(qint32 i);
    void readRawBytes(QByteArray &i, qint32 count);
    void writeRawBytes(QByteArray i);
    QByteArray toByteArray();
    void setByteOrder(QDataStream::ByteOrder order);

};

typedef TelegramStream TgStream;
typedef TelegramStream TgPacket;
typedef QVariantMap TelegramObject;

typedef QVariant TgVariant;
typedef QVariantMap TgObject;
typedef QVariantMap TgMap;
typedef QVariantList TgVector;
typedef QVariantList TgList;
typedef QVariantList TgArray;
typedef QByteArray TgInt128;
typedef QByteArray TgInt256;
typedef qint32 TgInt;
typedef qint32 TgInteger;
typedef qint64 TgLong;
typedef TgVariant TgLongVariant;
typedef long double TgDouble;
typedef QString TgString;
typedef bool TgBool;
typedef QByteArray TgByteArray;

typedef void (*WRITE_METHOD)(TelegramStream&, QVariant, void*);
typedef void (*READ_METHOD)(TelegramStream&, QVariant&, void*);

void readUInt8(TelegramStream& stream, QVariant &i, void* callback = 0);
void readUInt32(TelegramStream& stream, QVariant &i, void* callback = 0);
void readUInt64(TelegramStream& stream, QVariant &i, void* callback = 0);
void readInt32(TelegramStream& stream, QVariant &i, void* callback = 0);
void readInt64(TelegramStream& stream, QVariant &i, void* callback = 0);
void readDouble(TelegramStream& stream, QVariant &i, void* callback = 0);
void readBool(TelegramStream& stream, QVariant &i, void* callback = 0);
void readString(TelegramStream& stream, QVariant &i, void* callback = 0);
void readByteArray(TelegramStream& stream, QVariant &i, void* callback = 0);
void readInt128(TelegramStream& stream, QVariant &i, void* callback = 0);
void readInt256(TelegramStream& stream, QVariant &i, void* callback = 0);
void readVector(TelegramStream& stream, QVariant &i, void* callback);
void readList(TelegramStream& stream, QVariant &i, void* callback);

void writeUInt8(TelegramStream& stream, QVariant i, void* callback = 0);
void writeUInt32(TelegramStream& stream, QVariant i, void* callback = 0);
void writeUInt64(TelegramStream& stream, QVariant i, void* callback = 0);
void writeInt32(TelegramStream& stream, QVariant i, void* callback = 0);
void writeInt64(TelegramStream& stream, QVariant i, void* callback = 0);
void writeDouble(TelegramStream& stream, QVariant i, void* callback = 0);
void writeBool(TelegramStream& stream, QVariant i, void* callback = 0);
void writeString(TelegramStream& stream, QVariant i, void* callback = 0);
void writeByteArray(TelegramStream& stream, QVariant i, void* callback = 0);
void writeInt128(TelegramStream& stream, QVariant i, void* callback = 0);
void writeInt256(TelegramStream& stream, QVariant i, void* callback = 0);
void writeVector(TelegramStream& stream, QVariant i, void* callback);
void writeList(TelegramStream& stream, QVariant i, void* callback);

void readRawBytes(TelegramStream& stream, QByteArray &i, qint32 count);
void skipRawBytes(TelegramStream& stream, qint32 count);
void writeRawBytes(TelegramStream& stream, QByteArray i);

#define ID_PROPERTY(name) \
    name["_"]

#define TGOBJECT(id, name) \
    TgObject name;  \
    ID_PROPERTY(name) = id;

#define ID(name) \
    ID_PROPERTY(name).toInt()

#define GETID(name) \
    ID(name)

#define EMPTY(name) \
    (ID(name) == 0)

#define EXISTS(name) \
    (ID(name) != 0)

#define INT32_BYTES 4
#define INT64_BYTES 8
#define INT128_BYTES 16
#define INT256_BYTES 32

//TODO: use TLType

#define VECTOR_ID 481674261
#define BOOL_TRUE -1720552011
#define BOOL_FALSE -1132882121

template <WRITE_METHOD W> QByteArray tlSerialize(QVariant obj)
{
    TgPacket packet;
    (*W)(packet, obj, 0);
    return packet.toByteArray();
}

template <READ_METHOD R> QVariant tlDeserialize(QByteArray array)
{
    TgVariant obj;
    TgPacket packet(array);
    (*R)(packet, obj, 0);
    return obj;
}

QByteArray readFully(QIODevice &socket, qint32 length);

#endif // TELEGRAMSTREAM_H
