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

#include <QTextStream>

#include "attributelistiterator.h"
#include "protocoltreenode.h"
#include "protocoltreenodelistiterator.h"

ProtocolTreeNode::ProtocolTreeNode()
{
}

ProtocolTreeNode::~ProtocolTreeNode()
{
}

ProtocolTreeNode::ProtocolTreeNode(QString tag)
{
    this->tag = tag;
}

ProtocolTreeNode::ProtocolTreeNode(QString tag, QByteArray data)

{
    this->tag = tag;
    this->data = data;
}

void ProtocolTreeNode::addChild(ProtocolTreeNode& child)
{
    children.insert(child.getTag(),child);
}

void ProtocolTreeNode::setTag(QString tag)
{
    this->tag = tag;
}

void ProtocolTreeNode::setData(QByteArray data)
{
    this->data = data;
}

void ProtocolTreeNode::setAttributes(AttributeList attribs)
{
    attributes.clear();

    AttributeListIterator i(attribs);
    while (i.hasNext())
    {
        i.next();
        attributes.insert(i.key(),i.value());
    }
}

int ProtocolTreeNode::getAttributesCount()
{
    return attributes.size();
}

int ProtocolTreeNode::getChildrenCount()
{
    return children.size();
}

AttributeList& ProtocolTreeNode::getAttributes()
{
    return attributes;
}

const QString ProtocolTreeNode::getAttributeValue(QString key) const
{
    return attributes.value(key);
}

ProtocolTreeNodeList& ProtocolTreeNode::getChildren()
{
    return children;
}

ProtocolTreeNode ProtocolTreeNode::getChild(QString tag)
{
    return children.value(tag);
}

const QByteArray& ProtocolTreeNode::getData() const
{
    return data;
}

QString ProtocolTreeNode::getDataString()
{
    return QString::fromUtf8(data);
}


const QString& ProtocolTreeNode::getTag() const
{
    return tag;
}

QString ProtocolTreeNode::toString()
{
    QString result;
    QTextStream out(&result);

    out << "<" << tag << ">\n";
    out << attributes.toString();

    if (data.length() > 0)
        out << "   " +
               ((tag == "challenge" ||
                 tag == "response" ||
                 tag == "success" ||
                 tag == "auth" ||
                 tag == "picture" ||
                 tag == "media" )
                ? "Content of type " + tag + " length: " + QString::number(data.length())
                : data)
            << "\n";

    if (children.size() > 0)
    {
        ProtocolTreeNodeListIterator i(children);
        while (i.hasNext())
        {
            ProtocolTreeNode node = i.next().value();
            out << node.toString();
        }
    }

    return result;
}

void ProtocolTreeNode::setSize(int size)
{
    this->size = size;
}

int ProtocolTreeNode::getSize()
{
    return size;
}


