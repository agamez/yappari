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

#include <QApplication>
#include <QDataStream>
#include <QDateTime>

#include "ioexception.h"
#include "protocolexception.h"
#include "attributelist.h"
#include "util/utilities.h"
#include "bintreenodereader.h"

#define READ_TIMEOUT 30000

BinTreeNodeReader::BinTreeNodeReader(QTcpSocket *socket, QStringList& dictionary,
                                     QObject *parent) : QObject(parent)
{
    this->dictionary = dictionary;
    this->socket = socket;
}

int BinTreeNodeReader::getOneToplevelStream()
{
    qint32 bufferSize = readInt24();
    qint8 flags = (bufferSize & 0xff0000) >> 20;
    bufferSize &= 0xffff;

    fillBuffer(bufferSize);

    Utilities::logData("[[ " + readBuffer.toHex());
    decodeStream(flags, 0, bufferSize);

    return bufferSize + 3;
}

void BinTreeNodeReader::decodeStream(qint8 flags, qint32 offset, qint32 length)
{
    if ((flags & 8) != 0)
    {
        if (length < 4)
            throw new ProtocolException("Invalid length 0x" + QString::number(length,16));

        offset += 4;
        length -= 4;
        inputKey->decodeMessage(readBuffer, offset-4, offset, length);

        readBuffer = readBuffer.right(length);
        Utilities::logData("<< " + readBuffer.toHex());
    }
}

int BinTreeNodeReader::readStreamStart()
{
    int bytes = getOneToplevelStream();
    QDataStream in(readBuffer);

    quint8 tag, size;
    in >> tag;
    in >> size;
    in >> tag;
    if (tag != 1)
        throw ProtocolException("Expecting STREAM_START in readStreamStart.");

    int attribCount = (size - 2 + size % 2) / 2;

    AttributeList attribs;

    readAttributes(attribs,attribCount,in);

    return bytes;
}

bool BinTreeNodeReader::nextTree(ProtocolTreeNode& node)
{
    bool result;

    node.setSize(getOneToplevelStream());
    QDataStream in(readBuffer);

    result = nextTreeInternal(node, in);
    Utilities::logData(QDateTime::currentDateTime().toString());
    Utilities::logData(node.toString());
    return result;
}

bool BinTreeNodeReader::nextTreeInternal(ProtocolTreeNode& node, QDataStream &in)
{
    quint8 b;

    in >> b;
    int size = readListSize(b,in);
    in >> b;
    if (b == 2)
        return false;

    QByteArray tag;
    readString(b, tag, in);

    int attribCount = (size - 2 + size % 2) / 2;
    AttributeList attribs;
    readAttributes(attribs,attribCount,in);

    node.setTag(tag);
    node.setAttributes(attribs);

    if ((size % 2) == 1)
        return true;

    in >> b;
    if (isListTag(b))
    {
        readList(b,node,in);
        return true;
    }

    QByteArray data;
    readString(b,data,in);
    node.setData(data);
    return true;
}

bool BinTreeNodeReader::isListTag(quint32 b)
{
    return (b == 248) || (b == 0) || (b == 249);
}

void BinTreeNodeReader::readList(qint32 token,ProtocolTreeNode& node,QDataStream& in)
{
    int size = readListSize(token,in);
    for (int i=0; i<size; i++)
    {
        ProtocolTreeNode child;
        nextTreeInternal(child,in);
        node.addChild(child);
    }
}

void BinTreeNodeReader::readAttributes(AttributeList& attribs, quint32 attribCount,
                                       QDataStream& in)
{
    QByteArray key, value;
    for (quint32 i=0; i < attribCount; i++)
    {
        readString(key, in);
        readString(value, in);
        attribs.insert(QString::fromUtf8(key),QString::fromUtf8(value));
    }
}

quint32 BinTreeNodeReader::readListSize(qint32 token, QDataStream& in)
{
    int size;
    if (token == 0)
        size = 0;
    else if (token == 0xf8)
        size = readInt8(in);
    else if (token == 0xf9)
        size = readInt16(in);
    else
        throw ProtocolException("Invalid list size in readListSize: token 0x" + QString::number(token,16));

    return size;
}

void BinTreeNodeReader::fillBuffer(quint32 stanzaSize)
{
    fillArray(readBuffer, stanzaSize);
}

void BinTreeNodeReader::fillArray(QByteArray& buffer, quint32 len, QDataStream &in)
{
    buffer.clear();

    for (quint32 count=0; count < len; count ++)
    {
        quint8 byte;
        in >> byte;
        buffer.append(byte);
    }
}

void BinTreeNodeReader::fillArray(QByteArray& buffer, quint32 len)
{
    char data[1025];

    buffer.clear();

    // bool ready = true;

    /*
    if (socket->bytesAvailable() < 1)
    {
        Utilities::logData("fillArray() waiting for bytes");
        ready = socket->waitForReadyRead(READ_TIMEOUT);
    }

    if (!ready)
    {
        Utilities::logData("fillArray() not ready / timed out");
        throw IOException(socket->error());
    }
    */

    int needToRead = len;
    while (needToRead > 0)
    {
        int bytesRead = socket->read(data,(needToRead > 1024) ? 1024 : needToRead);

        if (bytesRead < 0)
            throw IOException(socket->error());
        if (bytesRead == 0)
            // socket->waitForReadyRead(READ_TIMEOUT);
            qApp->processEvents();
        else
        {
            needToRead -= bytesRead;
            buffer.append(data,bytesRead);
        }
    }
}

bool BinTreeNodeReader::readString(QByteArray& s, QDataStream& in)
{
    quint8 token;

    in >> token;
    return readString(token,s,in);
}

bool BinTreeNodeReader::readString(int token, QByteArray& s, QDataStream& in)
{
    int size;

    if (token == -1)
        throw ProtocolException("-1 token in readString.");

    if (token > 4 && token < 0xf5)
        return getToken(token,s);

    switch (token)
    {
        case 0:
            return false;

        case 0xfc:
            size = readInt8(in);
            fillArray(s,size,in);
            return true;

        case 0xfd:
            size = readInt24(in);
            fillArray(s,size,in);
            return true;

        case 0xfe:
            in >> token;
            return getToken(245 + token,s);

        case 0xfa:
            QByteArray user,server;
            bool usr = readString(user,in);
            bool srv = readString(server,in);
            if (usr & srv)
            {
                s = user + "@" + server;
                return true;
            }
            if (srv)
            {
                s = server;
                return true;
            }
            throw ProtocolException("readString couldn't reconstruct jid.");
    }
    throw ProtocolException("readString invalid token " + QString::number(token));
}

bool BinTreeNodeReader::getToken(int token, QByteArray &s)
{
    if (token >= 0 && token < dictionary.length())
    {
        s = dictionary.at(token).toUtf8();
        return true;
    }

    throw ProtocolException("Invalid token/length in getToken.");
}

quint8 BinTreeNodeReader::readInt8(QDataStream& in)
{
    quint8 result;

    in >> result;

    return(result);
}


qint32 BinTreeNodeReader::readInt16()
{
    // bool ready = true;
    char buffer[2];

    /*
    if (socket->bytesAvailable() < 2)
        ready = socket->waitForReadyRead(READ_TIMEOUT);

    if (!ready)
        throw IOException(socket->error());

    // Receive status code
    int bytesRead = socket->read(buffer,2);

    QByteArray readBytes(buffer,bytesRead);
    */

    QByteArray readBytes(buffer,2);

    fillArray(readBytes,2);

    qint32 result = (((quint8)readBytes.at(0)) << 8) + (quint8)readBytes.at(1);

    Utilities::logData("<< " + QString::number(result,16));

    return (result);
}

qint32 BinTreeNodeReader::readInt24()
{
    // bool ready = true;
    char buffer[3];

    /*
    if (socket->bytesAvailable() < 3)
        ready = socket->waitForReadyRead(READ_TIMEOUT);

    if (!ready)
        throw IOException(socket->error());

    // Receive status code
    int bytesRead = socket->read(buffer,3);

    QByteArray readBytes(buffer,bytesRead);

    */

    QByteArray readBytes(buffer,3);

    fillArray(readBytes,3);

    qint32 result = ((((quint8)readBytes.at(0)) << 16) + ((quint8)readBytes.at(1) << 8) +
                     ((quint8)readBytes.at(2)));

    Utilities::logData("<< " + QString::number(result,16));

    return (result);
}


qint16 BinTreeNodeReader::readInt16(QDataStream& in)
{
    quint8 r1;
    quint8 r2;
    qint32 result;

    in >> r1;
    in >> r2;

    result = (r1 << 8) + r2;

    return result;
}

qint32 BinTreeNodeReader::readInt24(QDataStream& in)
{
    quint8 r1;
    quint8 r2;
    quint8 r3;
    qint32 result;

    in >> r1;
    in >> r2;
    in >> r3;

    result = (r1 << 16) + (r2 << 8) + r3;

    return result;
}

void BinTreeNodeReader::setInputKey(KeyStream *inputKey)
{
    this->inputKey = inputKey;
}


