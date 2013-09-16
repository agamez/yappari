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

#include "util/utilities.h"
#include "util/qtrfc2898.h"

#include "protocolexception.h"

KeyStream::KeyStream(QByteArray key, QObject *parent) : QObject(parent)
{
    rc4 = new RC4(key,0x100);
    mac = new QtHmacSha1(key);
}

void KeyStream::decodeMessage(QByteArray& buffer, int macOffset, int offset, int length)
{
    QByteArray buffer2 = mac->hmacSha1(buffer, offset, length);

    for (int i = 0; i < 4; i++)
    {
        if (buffer[macOffset + i] != buffer2[i])
        {
            throw new ProtocolException("Invalid MAC");
        }
    }

    rc4->Cipher(buffer.data(),offset,length);
}


void KeyStream::encodeMessage(QByteArray &buffer, int macOffset, int offset, int length)
{
    rc4->Cipher(buffer.data(),offset,length);
    QByteArray hmac = mac->hmacSha1(buffer,offset,length);
    buffer.replace(macOffset, 4, hmac.constData(), 4);
}

QByteArray KeyStream::keyFromPasswordAndNonce(QByteArray& pass, QByteArray& nonce)
{
    QtRFC2898 bytes;

    return bytes.deriveBytes(pass, nonce, 0x10);
}




