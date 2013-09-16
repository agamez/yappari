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

#include "rc4.h"

#include "util/utilities.h"

RC4::RC4(QByteArray key, int drop)
{
    i = 0;

    char *key_data = key.data();

    while (i < LENGTH)
    {
        s[i] = i;
        i++;
    }
    j = 0;
    i = 0;

    while (i < LENGTH)
    {
        j = (uchar) ((j + s[i]) + key_data[i % key.size()]);

        int tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;

        i++;
    }
    i = j = 0;

    QByteArray dropArray(drop,0);

    Cipher(dropArray);

}

void RC4::Cipher(QByteArray data)
{
    Cipher(data.data(), 0, data.size());
}

void RC4::Cipher(char *data, int offset, int length)
{
    while (length-- != 0)
    {
        i = (i + 1) % 0x100;
        j = (j + s[i]) % 0x100;

        int tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;

        data[offset] = (uchar) (data[offset] ^ ((uchar) s[(s[i] + s[j]) % 0x100]));
        offset++;
    }
}
