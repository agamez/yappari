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

#include <QTextLayout>
#include <QTextDocument>
#include <QTextBlock>

#include "chattextitem.h"

#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/util/utilities.h"

#include "globalconstants.h"
#include "client.h"

ChatTextItem::ChatTextItem(FMessage message, QWidget *parent) :
    QLabel(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setAlignment(Qt::AlignTop|Qt::AlignLeft);
    setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Maximum);

    setTextFormat(Qt::RichText);
    setOpenExternalLinks(true);
    setWordWrap(true);

    setMessage(message);
}

void ChatTextItem::setMessage(FMessage message)
{
    this->message = message;
    setText(createHTML());
}

void ChatTextItem::updateTimestamp()
{
    setText(createHTML());
}

QString ChatTextItem::createHTML()
{
    bool from_me = message.key.from_me;

    QString day;


    if (!DateTimeUtilities::isSameDate(message.timestamp,
                                       QDateTime::currentMSecsSinceEpoch()))
        day = DateTimeUtilities::shortDayFormat(message.timestamp);


    QString time = DateTimeUtilities::shortTimeFormat(message.timestamp);

    QString from = (from_me)
            ? "You"
            : message.notify_name;

    QColor color = QMaemo5Style::standardColor("ActiveTextColor");

    QString nickcolor = Client::nickcolor.isEmpty() ? color.name() : Client::nickcolor;
    QString textcolor = Client::textcolor.isEmpty() ?
                QString() :
                "table { color: " + Client::textcolor + " } ";

    QString mycolor = Client::mycolor.isEmpty() ? "#333333" : Client::mycolor;

    QString html = "<style type=\"text/css\">"
                   ".nick { color:" + nickcolor + "; }"
                   "table { float:left; border-style:solid; border-width:1px; border-color:"
                   + mycolor +"; } "
                   + textcolor +
                   "</style>"
                   "<table width=\"100%\" border=\"1\" cellspacing=\"-1\"";

    if (from_me)
        html.append(" bgcolor=\"" + mycolor + "\"");

    html.append("><tr><td><table width=\"100%\" border=\"0\">"
                "<tr><td valign=\"center\"><span class=\"nick\">" +
                Utilities::WATextToHtml(from + ":",32) +
                "</span> " +
                Utilities::WATextToHtml(QString::fromUtf8(message.data),32) +
                "</td><td align=\"right\" valign=\"bottom\">");


    color = QMaemo5Style::standardColor("SecondaryTextColor");

    if (!day.isEmpty())
        html.append("<div style=\"font-size:18px;color:"
                    + color.name() +
                    "\">"
                    + day + " </div>");

    if (from_me)
    {
        if (message.status == FMessage::ReceivedByServer)
            html.append(CHECK);
        else if (message.status == FMessage::ReceivedByTarget)
           html.append(DOUBLECHECK);
        else
           html.append(GRAYCHECK);
    }

    html.append("<div style=\"font-size:18px;color:" + color.name() + "\">" + time +
                "</div></td></tr></table></td></tr></table>"
                "<div style=\"font-size:5px\">&nbsp;</div>");


/*
    // QTextLayout testing
    QString text = Utilities::formatMessage(QString::fromUtf8(message.data),32);
    QTextDocument doc;
    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    doc.setDefaultTextOption(option);
    doc.setUseDesignMetrics(true);
    doc.setHtml(text);
    doc.setTextWidth(543);
    QTextBlock block = doc.begin();
    Utilities::logData("========== DOCUMENT ==========\n" +
                       text + "\n" +
                       "Width: " + QString::number(doc.textWidth()) + "\n"
                       "Line Count: " + QString::number(doc.lineCount()) + "\n"
                       "Line Count: " + QString::number(block.lineCount()) + "\n");


    QTextLayout layout(text);
    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(option);
    layout.beginLayout();
    while (1)
    {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(543);
        Utilities::logData(text.mid(line.textStart(),line.textLength()) + "\n");
    }
    layout.endLayout();

    Utilities::logData("==== LAYOUT =====\n" +
                       QString::fromUtf8(message.data) + "\n-----------------\n"
                       "Maximum Width: " + QString::number(layout.maximumWidth()) + "\n"
                       "Minimum Width: " + QString::number(layout.minimumWidth()) + "\n"
                       "Line Count: " + QString::number(layout.lineCount()) + "\n"
                       "---------------\n" + layout.text());
*/

    return html;
}

FMessage ChatTextItem::getMessage()
{
    return message;
}
