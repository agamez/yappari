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

#include <QDateTime>

#include "util/utilities.h"
#include "attributelistiterator.h"
#include "protocoltreenodelistiterator.h"
#include "protocolexception.h"
#include "bintreenodewriter.h"


BinTreeNodeWriter::BinTreeNodeWriter(QTcpSocket *socket, QStringList& dictionary,
                                     QObject *parent) : QObject(parent)
{
    // Fill the token map dictionary
    for (int i = 0; i < dictionary.length(); i++)
        tokenMap.insert(dictionary.at(i),i);

    this->socket = socket;
    this->crypto = false;
}

/*
 *************************************************************************************
 * WRITER METHODS
 *************************************************************************************
 */

int BinTreeNodeWriter::streamStart(QString& domain, QString& resource)
{
    startBuffer();
    QDataStream out(&writeBuffer,QIODevice::WriteOnly);

    writeInt8(0x57, out);
    writeInt8(0x41, out);
    writeInt8(1, out);
    writeInt8(2, out);

    AttributeList streamOpenAttributes;
    streamOpenAttributes.insert("resource",resource);
    streamOpenAttributes.insert("to",domain);

    writeDummyHeader(out);
    writeListStart(streamOpenAttributes.size() * 2 + 1, out);
    writeInt8(1, out);
    writeAttributes(streamOpenAttributes, out);

    int bytes = writeBuffer.size();

    flushBuffer(false);

    return bytes;
}

void BinTreeNodeWriter::writeDummyHeader(QDataStream& out)
{
    dataBegin = writeBuffer.size();
    writeInt24(0, out);
}

/*
 * Buffer management methods
 */

void BinTreeNodeWriter::startBuffer()
{
    writeBuffer.clear();
    dataBegin = 0;
}

void BinTreeNodeWriter::processBuffer()
{
    int num = 0;

    if (crypto)
    {
        qint64 num2 = writeBuffer.size() + 4;
        writeBuffer.resize(num2);
        num |= 1;
    }

    qint64 num3 = writeBuffer.size() - 3 - dataBegin;
    if (num3 > 0x1000000)
        throw new ProtocolException("Buffer too large: " + QString::number(num3));

    if (crypto)
    {
        int length = ((int) num3) - 4;
        outputKey->encodeMessage(writeBuffer,(dataBegin + 3) + length, dataBegin + 3, length);
    }

    char *buffer = writeBuffer.data();
    buffer[dataBegin] = ((num << 4) | (num3 & 0xff0000) >> 0x10);
    buffer[dataBegin+1] = ((num3 & 0xff00) >> 8);
    buffer[dataBegin+2] = (num3 & 0xff);
}


void BinTreeNodeWriter::flushBuffer(bool flushNetwork)
{
    processBuffer();

    // Write buffer
    Utilities::logData(">> " + QString(writeBuffer.toHex()));
    if ((socket->write(writeBuffer)) == -1)
        throw IOException(socket->error());

    if (flushNetwork)
        socket->flush();

    writeBuffer.clear();
}

/*
 * Low level write methods
 */

void BinTreeNodeWriter::realWrite8(quint8 c)
{
    Utilities::logData(">> " + QString::number(c,16));
    if ((socket->write((char *)&c,1)) == -1)
        throw IOException(socket->error());
}

void BinTreeNodeWriter::realWrite16(quint16 data)
{
    realWrite8((quint8)((data & 0xFF00) >> 8));
    realWrite8((quint8)(data & 0xFF));
}


/*
 * High level write methods
 */

int BinTreeNodeWriter::write(ProtocolTreeNode& node, bool needsFlush)
{
    writeMutex.lock();
    startBuffer();
    QDataStream out(&writeBuffer,QIODevice::WriteOnly);

    writeDummyHeader(out);

    Utilities::logData(QDateTime::currentDateTime().toString());
    if (node.getTag() == "")
    {
        Utilities::logData("<noop>");
        writeInt8(0, out);
    }
    else
    {
        Utilities::logData(node.toString());
        writeInternal(node, out);
    }

    int bytes = writeBuffer.size();

    flushBuffer(needsFlush);
    writeMutex.unlock();

    return bytes;
}

void BinTreeNodeWriter::writeInternal(ProtocolTreeNode& node, QDataStream& out)
{
    writeListStart(1 + (node.getAttributesCount() * 2)
                   + (node.getChildrenCount() == 0 ? 0 : 1)
                   + (node.getData().length() == 0 ? 0 : 1), out);

    writeString(node.getTag(), out);
    writeAttributes(node.getAttributes(), out);
    if (node.getData().length() > 0)
        writeArray(node.getData(), out);
    if (node.getChildrenCount() > 0)
    {
        writeListStart(node.getChildrenCount(), out);
        ProtocolTreeNodeListIterator i(node.getChildren());
        while (i.hasNext())
        {
            ProtocolTreeNode child = i.next().value();
            writeInternal(child, out);
        }
    }
}

void BinTreeNodeWriter::writeListStart(qint32 i, QDataStream& out)
{
    if (i == 0)
    {
        writeInt8(0, out);
    }
    else if (i < 256)
    {
        writeInt8(248, out);
        writeInt8(i, out);
    }
    else
    {
        writeInt8(249, out);
        writeInt16(i, out);
    }
}


void BinTreeNodeWriter::writeAttributes(AttributeList& attributes, QDataStream &out)
{
    AttributeListIterator i(attributes);
    while (i.hasNext())
    {
        i.next();
        writeString(i.key(), out);
        writeString(i.value(), out);
    }
}

void BinTreeNodeWriter::writeString(QString tag, QDataStream& out)
{
    if (tag.size() == 0)
    {
        writeInt8(252, out);
        writeInt8(0, out);
    }
    else {

        int key = tokenMap.value(tag,-1);

        if (key != -1)
        {
            writeToken(key, out);
        }
        else
        {
            int atIndex = tag.indexOf('@');
            if (atIndex < 1)
            {
                writeArray(tag.toUtf8(), out);
            }
            else
            {
                QString server = tag.right(tag.length()-atIndex-1);
                QString user = tag.left(atIndex);
                writeJid(user, server, out);
            }
        }
    }
}

void BinTreeNodeWriter::writeJid(QString user, QString server, QDataStream& out)
{
    writeInt8(250, out);
    if (user.length() > 0)
        writeString(user, out);
    else
        writeToken(0, out);
    writeString(server, out);
}

void BinTreeNodeWriter::writeToken(qint32 intValue, QDataStream& out)
{
    if (intValue < 245)
        writeInt8(intValue, out);
    else if (intValue <= 500)
    {
        writeInt8(254, out);
        writeInt8(intValue - 245, out);
    }
}

void BinTreeNodeWriter::writeArray(QByteArray bytes, QDataStream& out)
{
    if (bytes.length() >= 256)
    {
        writeInt8(253, out);
        writeInt24(bytes.length(), out);
    }
    else
    {
        writeInt8(252, out);
        writeInt8(bytes.length(), out);
    }

    const char *constData = bytes.constData();
    for (int i=0; i < bytes.length(); i++)
        writeInt8((quint8) constData[i], out);
}

void BinTreeNodeWriter::writeInt8(quint8 v, QDataStream& out)
{
    out << v;
}

void BinTreeNodeWriter::writeInt16(quint16 v, QDataStream& out)
{
    out << v;
}

void BinTreeNodeWriter::writeInt24(quint32 v, QDataStream& out)
{
    out << ((quint8)((v & 0xFF0000) >> 16));
    out << ((quint16)((v & 0xFFFF)));
}

void BinTreeNodeWriter::setOutputKey(KeyStream *outputKey)
{
    this->outputKey = outputKey;
}

void BinTreeNodeWriter::setCrypto(bool crypto)
{
    this->crypto = crypto;
}
