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

#include "qtrfc2898.h"
#include "qthmacsha1.h"

#include "Whatsapp/protocolexception.h"

#define SHA1_DIGEST_LENGTH          20

QtRFC2898::QtRFC2898()
{
}

QByteArray QtRFC2898::deriveBytes(QByteArray& password, QByteArray& salt, int iterations)
{
    if (iterations == 0)
    {
        throw new ProtocolException("PBKDF2: Invalid iteration count");
    }

    if (password.length() == 0)
    {
        throw new ProtocolException("PBKDF2: Empty password is invalid");
    }

    QByteArray obuf, output;
    QByteArray asalt = salt;
    asalt.resize(salt.length() + 4);

    QByteArray key = password;
    int key_len = ( key.length() > SHA1_DIGEST_LENGTH ) ? SHA1_DIGEST_LENGTH : key.length();

    QtHmacSha1 hmacsha1(key);

    for (int count = 1; count < key_len && output.size() < key_len; count ++)
    {
        asalt[salt.length() + 0] = (count >> 24) & 0xff;
        asalt[salt.length() + 1] = (count >> 16) & 0xff;
        asalt[salt.length() + 2] = (count >> 8) & 0xff;
        asalt[salt.length() + 3] = count & 0xff;

        QByteArray d1 = hmacsha1.hmacSha1(asalt);
        obuf = d1;
        char *obuf_data = obuf.data();

        for (int i = 1; i < iterations; i++)
        {
            QByteArray d2 = hmacsha1.hmacSha1(d1);
            d1 = d2;
            for (int j = 0; j < obuf.length(); j++)
                obuf_data[j] ^= d1.at(j);
        }

        output.append(obuf);
    }

    return output.left(key_len);
}
