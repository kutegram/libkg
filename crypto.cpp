#include "crypto.h"

#include <QDateTime>
#include <QtCore>
#include "mtschema.h"
#include <mbedtls/aes.h>
#include <mbedtls/bignum.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha1.h>

qint64 randomLong()
{
    return qFromLittleEndian<qint64>((const uchar*) randomBytes(INT64_BYTES).constData());
}

DHKey::DHKey(QString publicKey, qint64 fingerprint, QString exponent) :
    publicKey(QByteArray::fromHex(publicKey.toLatin1())),
    exponent(QByteArray::fromHex(exponent.toLatin1())),
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
    array.resize((size / 4 + 1) * 4);

    qsrand(QDateTime::currentDateTime().toTime_t());
    int* data = (int*) array.data();

    for (qint32 i = 0; i < size / 4; ++i) {
        data[i] = qrand();
    }

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
    QByteArray result(a.size() > b.size() ? a : b);
    qint32 minLength = a.size() > b.size() ? b.size() : a.size();

    for (qint32 i = 0; i < minLength; ++i) {
        result[i] = (a[i] ^ b[i]);
    }

    return result;
}

QByteArray decryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key)
{
    QByteArray output;
    output.resize(data.size());

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_dec(&aes, (const unsigned char*) key.constData(), 256);
    mbedtls_aes_crypt_ige(&aes, MBEDTLS_AES_DECRYPT, data.size(), (unsigned char*) iv.data(), (const unsigned char*) data.constData(), (unsigned char*) output.data());
    mbedtls_aes_free(&aes);

    return output;
}

QByteArray encryptAES256IGE(QByteArray data, QByteArray iv, QByteArray key)
{
    QByteArray output;
    output.resize(data.size());

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char*) key.constData(), 256);
    mbedtls_aes_crypt_ige(&aes, MBEDTLS_AES_ENCRYPT, data.size(), (unsigned char*) iv.data(), (const unsigned char*) data.constData(), (unsigned char*) output.data());
    mbedtls_aes_free(&aes);

    return output;
}

QByteArray encryptRSA(QByteArray data, QByteArray key, QByteArray exp)
{
    mbedtls_mpi a, e, n, r;
    mbedtls_mpi_init(&a);
    mbedtls_mpi_init(&e);
    mbedtls_mpi_init(&n);
    mbedtls_mpi_init(&r);

    mbedtls_mpi_read_binary(&a, (const unsigned char*) data.constData(), data.size());
    mbedtls_mpi_read_binary(&e, (const unsigned char*) exp.constData(),  exp.size());
    mbedtls_mpi_read_binary(&n, (const unsigned char*) key.constData(),  key.size());

    QByteArray resultArray;

    qint32 result = mbedtls_mpi_exp_mod(&r, &a, &e, &n, 0);
    if (result != 0) {
        resultArray.resize(mbedtls_mpi_size(&r));
        mbedtls_mpi_write_binary(&r, (unsigned char*) resultArray.data(), resultArray.size());
    }

    mbedtls_mpi_free(&a);
    mbedtls_mpi_free(&e);
    mbedtls_mpi_free(&n);
    mbedtls_mpi_free(&r);

    return resultArray;
}

QByteArray hashSHA256(QByteArray dataToHash)
{
    QByteArray hash;
    hash.resize(32);

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, false);
    mbedtls_sha256_update(&ctx, (const unsigned char*) dataToHash.constData(), dataToHash.size());
    mbedtls_sha256_finish(&ctx, (unsigned char*) hash.data());
    mbedtls_sha256_free(&ctx);

    return hash;
}

QByteArray hashSHA1(QByteArray dataToHash)
{
    QByteArray hash;
    hash.resize(20);

    mbedtls_sha1_context ctx;
    mbedtls_sha1_init(&ctx);
    mbedtls_sha1_starts(&ctx);
    mbedtls_sha1_update(&ctx, (const unsigned char*) dataToHash.constData(), dataToHash.size());
    mbedtls_sha1_finish(&ctx, (unsigned char*) hash.data());
    mbedtls_sha1_free(&ctx);

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
