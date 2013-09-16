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
    QString lastStanza();

    void setInputKey(KeyStream *inputKey);

private:
    QStringList dictionary;
    QTcpSocket *socket;
    QByteArray readBuffer;
    KeyStream *inputKey;

    // Reader methods
    int getOneToplevelStream();
    void decodeStream(qint8 flags, qint32 offset, qint32 length);
    bool nextTreeInternal(ProtocolTreeNode& node, QDataStream &in);
    quint32 readListSize(qint32 token, QDataStream& in);
    void readList(qint32 token,ProtocolTreeNode& node,QDataStream& in);
    void fillBuffer(quint32 stanzaSize);
    void fillArray(QByteArray& buffer, quint32 len, QDataStream &in);
    void fillArray(QByteArray& buffer, quint32 len);
    bool isListTag(quint32 b);
    void readAttributes(AttributeList& attribs, quint32 attribCount,
                        QDataStream& in);
    bool readString(QByteArray& s, QDataStream& in);
    bool readString(qint32 token, QByteArray& s, QDataStream& in);
    bool getToken(qint32 token, QByteArray &s);
    quint8 readInt8(QDataStream& in);
    qint32 readInt16();
    qint32 readInt24();
    qint16 readInt16(QDataStream& in);
    qint32 readInt24(QDataStream& in);
};

#endif // BINTREENODEREADER_H
