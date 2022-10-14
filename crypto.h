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
QByteArray rsaPad(QByteArray data, DHKey key);

#endif // CRYPTO_H
