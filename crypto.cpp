#include "crypto.h"

#include <QDateTime>
#include <QCryptographicHash>
#include "mtschema.h"
#include <QtCore>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>

#ifdef Q_OS_SYMBIAN

extern unsigned int __aeabi_uidivmod(unsigned numerator, unsigned denominator);

int __aeabi_idiv(int numerator, int denominator)
{
    int neg_result = (numerator ^ denominator) & 0x80000000;
    int result = __aeabi_uidivmod ((numerator < 0) ? -numerator : numerator, (denominator < 0) ? -denominator : denominator);
    return neg_result ? -result : result;
}

unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator)
{
    return __aeabi_uidivmod (numerator, denominator);
}

#endif

DHKey::DHKey(QString publicKey, qint64 fingerprint, QString exponent) :
    publicKey(QByteArray::fromHex(publicKey.toAscii())),
    exponent(QByteArray::fromHex(exponent.toAscii())),
    fingerprint(fingerprint)
{
    if (!this->publicKey.isEmpty() && this->publicKey.at(0) == 0) {
        this->publicKey.remove(0, 1);
    }

    if (this->fingerprint == 0) {
        TgPacket packet;
        writeByteArray(packet, this->publicKey);
        writeByteArray(packet, this->exponent);

        QByteArray result = hashSHA1(packet.toByteArray()).mid(12, 8);
        this->fingerprint = qFromLittleEndian<qint64>((const uchar*) result.constData());
        qDebug() << this->fingerprint;
    }
}

qint32 randomInt(qint32 lowerThan)
{
    if (lowerThan < 1)
        return 0;
    return qAbs(qFromBigEndian<qint32>((uchar*) randomBytes(4).data())) % lowerThan;
}

QByteArray randomBytes(qint32 size)
{
    QByteArray array;
    array.resize(size);
    RAND_bytes((unsigned char*) array.data(), size);

    return array;
}

quint64 gcd(quint64 a, quint64 b) {
    if (a == 0) {
        return b;
    }
    if (b == 0) {
        return a;
    }

    int shift = 0;
    while ((a & 1) == 0 && (b & 1) == 0) {
        a >>= 1;
        b >>= 1;
        shift++;
    }

    while (true) {
        while ((a & 1) == 0) {
            a >>= 1;
        }
        while ((b & 1) == 0) {
            b >>= 1;
        }
        if (a > b) {
            a -= b;
        } else if (b > a) {
            b -= a;
        } else {
            return a << shift;
        }
    }
}

quint64 findDivider(quint64 number)
{
    qsrand(QDateTime::currentDateTime().toUTC().toTime_t());
    int it = 0;
    quint64 g = 0;
    for (int i = 0; i < 3 || it < 10000; ++i) {
        const quint64 q = ((qrand() & 15) + 17) % number;
        quint64 x = (quint64) qrand() % (number - 1) + 1;
        quint64 y = x;
        const quint32 lim = 1 << (i + 18);
        for (quint32 j = 1; j < lim; j++) {
            ++it;
            quint64 a = x;
            quint64 b = x;
            quint64 c = q;
            while (b) {
                if (b & 1) {
                    c += a;
                    if (c >= number) {
                        c -= number;
                    }
                }
                a += a;
                if (a >= number) {
                    a -= number;
                }
                b >>= 1;
            }
            x = c;
            const quint64 z = x < y ? number + x - y : x - y;
            g = gcd(z, number);
            if (g != 1) {
                return g;
            }
            if (!(j & (j - 1))) {
                y = x;
            }
        }

        if (g > 1 && g < number) {
            return g;
        }
    }

    return 1;
}

QByteArray reverse(QByteArray array)
{
    for (int low = 0, high = array.size() - 1; low < high; ++low, --high) {
        qSwap(array.data()[low], array.data()[high]);
    }
    return array;
}

QByteArray xorArray(QByteArray a, QByteArray b)
{
    QByteArray result(a.size() > b.size() ? a : b);
    qint32 minLength = a.size() > b.size() ? b.size() : a.size();

    for (qint32 i = 0; i < minLength; ++i) {
        result[i] = (a[i] ^ b[i]);
    }

    return result;
}

QByteArray decryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key)
{
    AES_KEY key_enc;
    AES_set_decrypt_key((const unsigned char*) key.constData(), 256, &key_enc);

    QByteArray outData;
    outData.resize(data.size());

    AES_ige_encrypt((const unsigned char*) data.constData(), (unsigned char*) outData.data(), data.size(), &key_enc, (unsigned char*) iv.data(), AES_DECRYPT);

    return outData;
}

QByteArray encryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key)
{
    AES_KEY key_enc;
    AES_set_encrypt_key((const unsigned char*) key.constData(), 256, &key_enc);

    QByteArray outData;
    outData.resize(data.size());

    AES_ige_encrypt((const unsigned char*) data.constData(), (unsigned char*) outData.data(), data.size(), &key_enc, (unsigned char*) iv.data(), AES_ENCRYPT);

    return outData;
}

QByteArray encryptRSA(QByteArray data, QByteArray key, QByteArray exp)
{
    //data ^ exp mod key
    BIGNUM* x = BN_new();
    BIGNUM* n = BN_new();
    BIGNUM* e = BN_new();
    BIGNUM* r = BN_new();
    BN_CTX* ctx = BN_CTX_new();

    BN_bin2bn((const unsigned char*) data.constData(), data.length(), x);
    BN_bin2bn((const unsigned char*) key.constData(), key.length(), n);
    BN_bin2bn((const unsigned char*) exp.constData(), exp.length(), e);
    int result = BN_mod_exp(r, x, e, n, ctx);

    QByteArray resultArray;
    if (result) {
        resultArray.resize(BN_num_bytes(r));
        BN_bn2bin(r, (unsigned char*) resultArray.data());
    }

    BN_free(x);
    BN_free(n);
    BN_free(e);
    BN_free(r);
    BN_CTX_free(ctx);

    return resultArray;
}

QByteArray hashSHA256(QByteArray dataToHash)
{
    QByteArray hash;
    hash.resize(SHA256_DIGEST_LENGTH);

    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, dataToHash.constData(), dataToHash.size());
    SHA256_Final((unsigned char*) hash.data(), &ctx);

    return hash;
}

QByteArray hashSHA1(QByteArray dataToHash)
{
    QByteArray hash;
    hash.resize(SHA_DIGEST_LENGTH);

    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, dataToHash.constData(), dataToHash.size());
    SHA1_Final((unsigned char*)hash.data(), &ctx);

    return hash;
}

QByteArray calcMessageKey(QByteArray authKey, QByteArray data)
{
    return hashSHA256(authKey.mid(88, 32) + data).mid(8, 16);
}

QByteArray calcEncryptionKey(QByteArray authKey, QByteArray msgKey, QByteArray &iv, bool client)
{
    qint32 x = client ? 0 : 8;

    QByteArray sha256A = hashSHA256(msgKey + authKey.mid(x, 36));
    QByteArray sha256B = hashSHA256(authKey.mid(40 + x, 36) + msgKey);

    iv = sha256B.mid(0, 8) + sha256A.mid(8, 16) + sha256B.mid(24, 8);

    return sha256A.mid(0, 8) + sha256B.mid(8, 16) + sha256A.mid(24, 8);
}

qint8 compareAsBigEndian(QByteArray a, QByteArray b)
{
    if (a.length() != b.length())
        return a.length() > b.length() ? 1 : -1;

    for (qint32 i = 0; i < a.length(); ++i) {
        if ((quint8) a[i] > (quint8) b[i])
            return 1;
        if ((quint8) b[i] > (quint8) a[i])
            return -1;
    }

    return 0;
}

QByteArray rsaPad(QByteArray data, DHKey key)
{
    QByteArray dataWithPadding = data + randomBytes(192 - data.length());
    QByteArray dataPadReversed = reverse(dataWithPadding);

    QByteArray keyAesEncrypted;
    do {
        QByteArray tempKey = randomBytes(32);
        QByteArray dataWithHash = dataPadReversed + hashSHA256(tempKey + dataWithPadding);
        QByteArray aesEncrypted = encryptAES256IGE(dataWithHash, QByteArray(32, 0), tempKey);
        QByteArray tempKeyXor = xorArray(tempKey, hashSHA256(aesEncrypted));
        keyAesEncrypted = tempKeyXor + aesEncrypted;
    } while (compareAsBigEndian(keyAesEncrypted, key.publicKey) != -1);

    return encryptRSA(keyAesEncrypted, key.publicKey, key.exponent);
}
