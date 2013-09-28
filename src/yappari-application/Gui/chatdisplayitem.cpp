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

#include <QMaemo5Style>

#include <QTextDocument>
#include <QFontMetrics>
#include <QAbstractTextDocumentLayout>

#include "chatdisplayitem.h"

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"

#define TEXT_WIDTH      700

ChatDisplayItem::ChatDisplayItem(Contact *c) :
    QStandardItem()
{
    this->contact = c;
    muted = false;
    muteExpireTimestamp = 0;

    setEditable(false);

    setData(c->jid,Qt::UserRole + 1);
    setData(QDateTime::currentMSecsSinceEpoch(), Qt::UserRole + 2);

    updatePhoto(contact);

    updateData();
}

void ChatDisplayItem::updatePhoto(Contact *c)
{
    if (c->photoId.isEmpty())
    {
        if (c->type == Contact::TypeGroup)
            setData(QImage("/usr/share/yappari/icons/64x64/general_conference_avatar.png"), Qt::UserRole);
        else
            setData(QImage("/usr/share/icons/hicolor/64x64/hildon/general_default_avatar.png"), Qt::UserRole);
    }
    else
        setData(c->photo, Qt::UserRole);
}

void ChatDisplayItem::updateData(FMessage& msg)
{
    lastMessage = msg;
    updateData();
}

void ChatDisplayItem::updateData()
{
    setData(lastMessage.timestamp,Qt::UserRole + 2);

    QString htmlLastLine;
    QTextDocument doc,line;
    qreal height, lineHeight;

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");

    line.setHtml("<div style=\"font-size:18px;color:" + color.name() + "\">O</div>");

    QString lastLine;

    if (lastMessage.type == FMessage::UndefinedMessage)
        lastLine = "";
    else
    {
        if (lastMessage.type == FMessage::BodyMessage)
            lastLine = QString::fromUtf8(lastMessage.data);
        else if (lastMessage.type == FMessage::MediaMessage && lastMessage.live)
            lastLine = "<i>[voice note]</i>";
        else
            lastLine = "<i>[" + lastMessage.getMediaWAType() + "]</i>";

    }

    lastLine.replace("\n"," ");
    if (lastLine.length() > 80)
        lastLine = lastLine.left(80) + "...";

    do {
        htmlLastLine = "<div style=\"font-size:18px;color:" + color.name() + "\">" +
                       lastLine + "</div>";

        doc.setTextWidth(TEXT_WIDTH);
        doc.setHtml(htmlLastLine);
        height = doc.documentLayout()->documentSize().height();
        lineHeight = line.documentLayout()->documentSize().height();

        if (height > lineHeight)
        {
            int spaceIndex = lastLine.lastIndexOf(" ");
            lastLine = lastLine.left( (spaceIndex >= 0) ? spaceIndex : lastLine.length() - 10 ) + "...";
        }

    } while (lastLine.length() > 0 && height > lineHeight);

    QString timeString = (DateTimeUtilities::isSameDate(lastMessage.timestamp,QDateTime::currentMSecsSinceEpoch()))
            ? DateTimeUtilities::shortTimeFormat(lastMessage.timestamp)
            : DateTimeUtilities::shortDayFormat(lastMessage.timestamp);

    htmlLastLine = "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\"><tr><td>"
                   "<div style=\"font-size:18px;color:" + color.name() + "\">" +
                   ((lastMessage.type == FMessage::BodyMessage) ?
                        Utilities::WATextToHtml(lastLine,lineHeight - 8) :
                        lastLine)
                   + "</div>"
                   "</td></tr></table>";

    QString html = "<table width=\"100%\" border=\"0\" cellspacing=\"0\" cellpadding=\"0\"><tr><td>" +
                   Utilities::WATextToHtml(contact->name, 32) +
                   "</td><td align=\"right\"><div style=\"font-size:18px;color:" +
                   color.name() + "\">" +
                   timeString +
                   "&nbsp;&nbsp;</td></tr></table>"
                   + htmlLastLine;
    setData(html,Qt::DisplayRole);
}

