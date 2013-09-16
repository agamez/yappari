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

#ifndef PROTOCOLTREENODE_H
#define PROTOCOLTREENODE_H

#include <QObject>
#include <QString>
#include <QMap>

#include "attributelist.h"
#include "protocoltreenodelist.h"

class ProtocolTreeNode
{

public:
    ProtocolTreeNode();
    ~ProtocolTreeNode();
    ProtocolTreeNode(QString tag);
    ProtocolTreeNode(QString tag, QByteArray data);

    void addChild(ProtocolTreeNode& child);
    void setTag(QString tag);
    void setData(QByteArray data);
    void setAttributes(AttributeList attribs);
    void setSize(int size);

    int getSize();
    int getAttributesCount();
    int getChildrenCount();
    const QByteArray& getData() const;
    QString getDataString();
    const QString& getTag() const;
    const QString getAttributeValue(QString key) const;
    AttributeList& getAttributes();
    ProtocolTreeNodeList& getChildren();
    ProtocolTreeNode getChild(QString tag);
    QString toString();

private:
    QString tag;
    QByteArray data;
    AttributeList attributes;
    ProtocolTreeNodeList children;
    int size;

};

#endif // PROTOCOLTREENODE_H
