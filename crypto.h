#ifndef CRYPTO_H
#define CRYPTO_H

#include <QByteArray>
#include <QString>
#include "telegramstream.h"

struct DHKey
{
    QByteArray publicKey;
    QByteArray exponent;
    qint64 fingerprint;

    DHKey(QString publicKey, qint64 fingerprint, QString exponent = "010001");
};

//DHKey("c150023e2f70db7985ded064759cfecf0af328e69a41daf4d6f01b538135a6f91f8f8b2a0ec9ba9720ce352efcf6c5680ffc424bd634864902de0b4bd6d49f4e580230e3ae97d95c8b19442b3c0a10d8f5633fecedd6926a7f6dab0ddb7d457f9ea81b8465fcd6fffeed114011df91c059caedaf97625f6c96ecc74725556934ef781d866b34f011fce4d835a090196e9a5f0e4449af7eb697ddb9076494ca5f81104a305b6dd27665722c46b60e5df680fb16b210607ef217652e60236c255f6a28315f4083a96791d7214bf64c1df4fd0db1944fb26a2a57031b32eee64ad15a8ba68885cde74a5bfc920f6abf59ba5c75506373e7130f9042da922179251f", -4344800451088585951LL);

qint32 randomInt(qint32 lowerThan);
QByteArray randomBytes(qint32 size);
quint64 findDivider(quint64 number);
QByteArray reverse(QByteArray array);
QByteArray xorArray(QByteArray a, QByteArray b);
QByteArray decryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key);
QByteArray encryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key);
QByteArray encryptRSA(QByteArray data, QByteArray key, QByteArray exp);
QByteArray hashSHA256(QByteArray dataToHash);
QByteArray hashSHA1(QByteArray dataToHash);
QByteArray calcMessageKey(QByteArray authKey, QByteArray data);
QByteArray calcEncryptionKey(QByteArray sharedKey, QByteArray msgKey, QByteArray &iv, bool client);

#endif // CRYPTO_H
