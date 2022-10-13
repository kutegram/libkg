#include "crypto.h"

#include <QDateTime>
#include "thirdparty/picosha2.h"
#include "thirdparty/aes.hpp"
#include <QCryptographicHash>
#include "mtschema.h"
#include <QtCore>
#include <openssl/bn.h>
#include <openssl/rsa.h>

DHKey::DHKey(QString publicKey, qint64 fingerprint, QString exponent) :
    publicKey(QByteArray::fromHex(publicKey.toUtf8())),
    exponent(QByteArray::fromHex(exponent.toUtf8())),
    fingerprint(fingerprint)
{

}

qint32 randomInt(qint32 lowerThan) {
    if (lowerThan < 1) return 0;
    return qAbs(qFromBigEndian<qint32>((uchar*) randomBytes(4).data())) % lowerThan;
}

QByteArray randomBytes(qint32 size)
{
    //TODO: use OpenSSL's secure random
    QByteArray array;

    qsrand(QDateTime::currentDateTime().toUTC().toTime_t());
    array.reserve(size);
    while (array.length() < size) array.append((quint8) (qrand() % 256));
    array.resize(size);

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
    QByteArray result(a.length(), 0);
    for (qint32 i = 0; i < a.length(); ++i) {
        result[i] = (a[i] ^ b[i]);
    }
    return result;
}

QByteArray decryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key)
{
    QByteArray result(data);
    AES_ctx ctx;
    AES_init_ctx_iv32(&ctx, (const uint8_t*) key.constData(), (const uint8_t*) iv.constData());
    AES_IGE_decrypt_buffer(&ctx, (uint8_t*) result.data(), result.size());
    return result;
}

QByteArray encryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key)
{
    QByteArray result(data);
    AES_ctx ctx;
    AES_init_ctx_iv32(&ctx, (const uint8_t*) key.constData(), (const uint8_t*) iv.constData());
    AES_IGE_encrypt_buffer(&ctx, (uint8_t*) result.data(), result.size());
    return result;
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

    QByteArray resultArray(BN_num_bytes(r), 0);
    if (result) {
        BN_bn2bin(r, (unsigned char*) resultArray.data());
    } else resultArray.clear();

    BN_free(x);
    BN_free(n);
    BN_free(e);
    BN_free(r);
    BN_CTX_free(ctx);

    return resultArray;
}

QByteArray hashSHA256(QByteArray dataToHash)
{
    QByteArray dataHash(32, 0);
    picosha2::hash256(dataToHash.begin(), dataToHash.end(), dataHash.begin(), dataHash.end());
    return dataHash;
}

QByteArray hashSHA1(QByteArray dataToHash)
{
    return QCryptographicHash::hash(dataToHash, QCryptographicHash::Sha1);
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
