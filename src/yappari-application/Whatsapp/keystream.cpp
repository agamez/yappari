/* Copyright 2013 Naikel Aparicio. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the author and should not be interpreted as representing
 * official policies, either expressed or implied, of the copyright holder.
 */

#include "keystream.h"
#include "protocolexception.h"

#include "util/utilities.h"
#include "util/qtrfc2898.h"

KeyStream::KeyStream(QByteArray rc4key, QByteArray mackey, QObject *parent) : QObject(parent)
{
    rc4 = new RC4(rc4key,0x300);
    //qDebug() << "RC4 key:" << rc4key.toHex();
    mac = new QtHmacSha1(mackey);
    //qDebug() << "HMAC key:" << mackey.toHex();
    seq = 0;
}

bool KeyStream::decodeMessage(QByteArray& buffer, int macOffset, int offset, int length)
{
    //qDebug() << "decodeMessage seq:" << seq;
    QByteArray base = buffer.left(buffer.size() - 4);
    QByteArray hmac = buffer.right(4);
    QByteArray buffer2 = processBuffer(base, seq++);
    buffer2 = mac->hmacSha1(buffer2);

    QByteArray origBuffer = buffer;
    buffer = base;
    rc4->Cipher(buffer.data(),0,buffer.size());

    for (int i = 0; i < 4; i++)
    {
        if (buffer2[macOffset + i] != hmac[i])
        {
            qDebug() << "error decoding message. macOffset:" << macOffset << "offset:" << offset << "length:" << length << "bufferSize:" << buffer.size();
            qDebug() << "buffer mac:" << buffer2.toHex() << "hmac:" << hmac.toHex();
            qDebug() << "buffer:" << buffer.toHex();
            qDebug() << "origBuffer:" << origBuffer.toHex();
            return false;
        }
    }
    return true;
}


void KeyStream::encodeMessage(QByteArray &buffer, int macOffset, int offset, int length, bool dout)
{
    //qDebug() << "encodeMessage seq:" << seq;
    rc4->Cipher(buffer.data(),offset,length);
    QByteArray base = buffer.mid(offset,length);
    base = processBuffer(base, seq++);
    QByteArray hmac = mac->hmacSha1(base);
    hmac.resize(4);

    buffer.replace(macOffset, 4, hmac.constData(), 4);
}

QList<QByteArray> KeyStream::keyFromPasswordAndNonce(QByteArray& pass, QByteArray& nonce)
{
    QList<QByteArray> keys;

    QtRFC2898 bytes;

    for (int i = 1; i < 5; i++) {
        QByteArray nnonce = nonce;
        nnonce.append(i);
        keys.append(bytes.deriveBytes(pass, nnonce, 2));
    }

    return keys;
}

QByteArray KeyStream::processBuffer(QByteArray buffer, int seq)
{
    buffer.append(seq >> 0x18);
    buffer.append(seq >> 0x10);
    buffer.append(seq >> 0x8);
    buffer.append(seq);
    return buffer;
}
