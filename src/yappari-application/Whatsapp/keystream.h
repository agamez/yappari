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

#ifndef KEYSTREAM_H
#define KEYSTREAM_H

#include <QObject>
#include <QByteArray>

#include "util/qthmacsha1.h"
#include "rc4.h"

class KeyStream : public QObject
{
    Q_OBJECT

public:
    explicit KeyStream(QByteArray rc4key, QByteArray mackey, QObject *parent = 0);

    bool decodeMessage(QByteArray& buffer, int macOffset, int offset, int length);
    void encodeMessage(QByteArray& buffer, int macOffset, int offset, int length, bool dout = true);

    static QList<QByteArray> keyFromPasswordAndNonce(QByteArray& pass, QByteArray& nonce);
    static QByteArray deriveBytes(QByteArray& password, QByteArray& salt, int iterations);

private:
    QByteArray processBuffer(QByteArray buffer, int seq = 0);

    RC4 *rc4;
    QtHmacSha1 *mac;
    int seq;

};

#endif // KEYSTREAM_H
