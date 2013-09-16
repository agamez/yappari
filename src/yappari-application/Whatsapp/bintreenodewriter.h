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

#ifndef BINTREENODEWRITER_H
#define BINTREENODEWRITER_H

#include <QDataStream>
#include <QStringList>
#include <QTcpSocket>
#include <QMutex>

#include "keystream.h"
#include "ioexception.h"
#include "attributelist.h"
#include "protocoltreenodelist.h"

class BinTreeNodeWriter : public QObject
{
    Q_OBJECT

public:

    BinTreeNodeWriter(QTcpSocket *socket, QStringList& dictionary,
                      QObject *parent = 0);

    // Writer methods
    int write(ProtocolTreeNode& node, bool needsFlush = true);
    int streamStart(QString& domain, QString& resource);

    void setOutputKey(KeyStream *outputKey);
    void setCrypto(bool crypto);

private:
    QHash<QString, int> tokenMap;
    QTcpSocket *socket;
    QByteArray writeBuffer;
    QMutex writeMutex;
    qint32 dataBegin;
    KeyStream *outputKey;
    bool crypto;

    // Writer methods
    void startBuffer();
    void processBuffer();
    void flushBuffer(bool flushNetwork);
    void realWrite8(quint8 c);
    void realWrite16(quint16 data);
    void writeDummyHeader(QDataStream& out);
    void writeInternal(ProtocolTreeNode& node, QDataStream& out);
    void writeListStart(qint32 i, QDataStream& out);
    void writeAttributes(AttributeList& attributes, QDataStream& out);
    void writeString(QString tag, QDataStream& out);
    void writeJid(QString user, QString server, QDataStream& out);
    void writeToken(qint32 intValue, QDataStream& out);
    void writeArray(QByteArray bytes, QDataStream& out);
    void writeInt8(quint8 v, QDataStream& out);
    void writeInt16(quint16 v, QDataStream& out);
    void writeInt24(quint32 v, QDataStream& out);
};

#endif // BINTREENODEWRITER_H
