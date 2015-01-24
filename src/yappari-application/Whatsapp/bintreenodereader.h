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

#ifndef BINTREENODEREADER_H
#define BINTREENODEREADER_H

#include <QDataStream>
#include <QBuffer>
#include <QStringList>
#include <QTcpSocket>

#include "keystream.h"
#include "attributelist.h"
#include "protocoltreenode.h"
#include "protocoltreenodelist.h"

class BinTreeNodeReader : public QObject
{
    Q_OBJECT

public:

    // Constructor
    BinTreeNodeReader(QTcpSocket *socket, QStringList& dictionary,
                      QObject *parent = 0);

    // Reader methods
    int readStreamStart();
    bool nextTree(ProtocolTreeNode& node);

    void setInputKey(KeyStream *inputKey);

private:
    QStringList dictionary;
    QTcpSocket *socket;
    KeyStream *inputKey;

    QByteArray rawBuffer;
    QByteArray decodedBuffer;
    QBuffer decodedStream;

    // Reader methods
    int getOneToplevelStreamSize();
    bool getOneToplevelStream();
    bool decodeRawStream(qint8 flags, qint32 offset, qint32 length);

    //Raw stream reads
    bool fillRawBuffer(quint32 stanzaSize);
    bool fillArrayFromRawStream(QByteArray& buffer, quint32 len);

    qint32 readRawInt16();
    qint32 readRawInt24();

    //Decoded stream reads
    bool nextTreeInternal(ProtocolTreeNode& node);
    bool readListSize(qint32 token, int &size);
    bool readList(qint32 token,ProtocolTreeNode& node);

    bool fillArray(QByteArray& buffer, quint32 len);
    bool readAttributes(AttributeList& attribs, int attribCount);
    bool readString(QByteArray& s);
    bool readString(qint32 token, QByteArray& s);
    bool getToken(qint32 token, QByteArray &s);

    bool readInt8(quint8 &val);
    bool readInt16(qint16 &val);
    bool readInt24(qint32 &val);

    //helper functions
    bool isListTag(quint32 b);

};

#endif // BINTREENODEREADER_H
