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
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Eeli Reilin.
 */

#include <QApplication>
#include <QTimer>

#include "globalconstants.h"
#include "chatimageitem.h"
#include "chattextitem.h"
#include "client.h"
#include "chatarea.h"

#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/util/utilities.h"

ChatArea::ChatArea(QWidget *parent) :
    QScrollArea(parent)
{
}

void ChatArea::init()
{
    // Initialization has to be done after all the UI setup

    container = widget();
    grid = new QVBoxLayout(container);
    grid->setAlignment(Qt::AlignTop);
    grid->setContentsMargins(QMargins(0,0,0,0));
    setWidgetResizable(true);
    setWidget(container);

    connect(verticalScrollBar(),SIGNAL(rangeChanged(int,int)),
            this,SLOT(sliderRangeChanged(int,int)));

    connect(verticalScrollBar(),SIGNAL(valueChanged(int)),
            this,SLOT(sliderMoved(int)));

    loadingMessages = false;
}

void ChatArea::sliderMoved(int value)
{
    if (value == 0)
        emit topReached();
}


void ChatArea::sliderRangeChanged(int min, int max)
{
    Q_UNUSED(min);

    if (!loadingMessages)
    {
        Utilities::logData("Autoscrolling");
        verticalScrollBar()->setValue(max);
    }
}

void ChatArea::insertMessageAtTop(FMessage message)
{
    insertMessage(message,true);
}

void ChatArea::insertMessageAtBottom(FMessage message)
{
    insertMessage(message,false);
}

void ChatArea::insertMessage(FMessage message, bool atTop)
{
    if (message.type == FMessage::BodyMessage)
        insertBodyMessage(message,atTop);
    if (message.type == FMessage::MediaMessage)
        insertMediaMessage(message,atTop);
}

void ChatArea::insertBodyMessage(FMessage message, bool atTop)
{
    ChatTextItem *textItem = new ChatTextItem(message,container);
    connect(this,SIGNAL(updateTimestamps()),textItem,SLOT(updateTimestamp()));
    QWidget *label = textItem;

    if (!atTop)
    {
        grid->addWidget(label);
        widgetList.append(label);
    }
    else
    {
        if (!widgetAtBottom)
            widgetAtBottom = label;
        grid->insertWidget(0,label);
        widgetList.insert(0,label);
    }

    widgets.insert(message.key,label);
}

void ChatArea::insertMediaMessage(FMessage message, bool atTop)
{
    QWidget *label = 0;

    if (message.media_wa_type == FMessage::Image ||
        message.media_wa_type == FMessage::Video ||
        message.media_wa_type == FMessage::Audio)
    {
        ChatImageItem *imageItem = new ChatImageItem(message,container);
        connect(this,SIGNAL(updateTimestamps()),imageItem,SLOT(updateTimestamp()));
        connect(imageItem,SIGNAL(mediaDownload(FMessage)),
                this,SLOT(mediaDownloadHandler(FMessage)));
        label = imageItem;
    }

    if (label != 0)
    {
        if (!atTop)
        {
            grid->addWidget(label);
            widgetList.append(label);
        }
        else
        {
            if (!widgetAtBottom)
                widgetAtBottom = label;
            grid->insertWidget(0,label);
            widgetList.insert(0,label);
        }

        widgets.insert(message.key,label);
    }
}

void ChatArea::mediaDownloadHandler(FMessage message)
{
    emit mediaDownload(message);
}

void ChatArea::updateStatus(FMessage message)
{
    if (widgets.contains(message.key))
    {
        if (message.type == FMessage::BodyMessage)
        {
            // QLabel *label = (QLabel *)widgets.value(message.key);
            // label->setText(createHTML(message));

            ChatTextItem *label = (ChatTextItem *)widgets.value(message.key);
            label->setMessage(message);
        }
        else if (message.type == FMessage::MediaMessage)
        {
            ChatImageItem *label = (ChatImageItem *)widgets.value(message.key);
            label->setTimestamp(message);
        }

    }
}

void ChatArea::updateProgress(FMessage message, float p)
{
    if (widgets.contains(message.key) && message.type == FMessage::MediaMessage)
    {
        ChatImageItem *label = (ChatImageItem *)widgets.value(message.key);
        label->updateProgress(p);
    }
}

void ChatArea::updateUri(FMessage message)
{
    if (widgets.contains(message.key) && message.type == FMessage::MediaMessage)
    {
        ChatImageItem *label = (ChatImageItem *)widgets.value(message.key);
        label->updateUri(message.local_file_uri);
    }
}

void ChatArea::updateImage(FMessage message)
{
    if (widgets.contains(message.key) && message.type == FMessage::MediaMessage)
    {
        ChatImageItem *label = (ChatImageItem *)widgets.value(message.key);
        label->updateImage(message);
    }
}


void ChatArea::resetButton(FMessage message)
{
    if (widgets.contains(message.key) && message.type == FMessage::MediaMessage)
    {
        ChatImageItem *label = (ChatImageItem *)widgets.value(message.key);
        label->resetButton();
    }
}


QString ChatArea::createHTML(FMessage message)
{
    bool from_me = message.key.from_me;

    QString day;


    if (!DateTimeUtilities::isSameDate(message.timestamp,
                                       QDateTime::currentMSecsSinceEpoch()))
        day = DateTimeUtilities::shortDayFormat(message.timestamp);


    QString time = DateTimeUtilities::shortTimeFormat(message.timestamp);

    QString from = (from_me)
            ? "You"
            : message.notify_name.replace("<","&lt;").replace(">","&gt;");

    QString nickcolor = Client::nickcolor.isEmpty() ? "cyan" : Client::nickcolor;
    QString textcolor = Client::textcolor.isEmpty() ?
                QString() :
                "table { color: " + Client::textcolor + " } ";
    QString mycolor = Client::mycolor.isEmpty() ? "#333333" : Client::mycolor;

    QString html = "<style type=\"text/css\">"
                   "table { border-style:solid; border-width:1px; border-color:"
                   + mycolor +"; } "
                   + textcolor +
                   "</style>"
                   "<table width=\"100%\" border=\"1\" cellspacing=\"-1\"";

    if (from_me)
        html.append(" bgcolor=\"" + mycolor + "\"");

    //html.append("><tr><td><table width=\"100%\" border=\"0\"><tr><td valign=\"bottom\"><div style=\"color:cyan\">" +
    //            from + ":</div> " + processMessage(message.data) +
    //            "</td><td align=\"right\" valign=\"bottom\">");


    html.append("><tr><td><table width=\"100%\" border=\"0\">"
                "<tr><td valign=\"center\"><div style=\"color:"
                + nickcolor + "\">" +
                from + ":</div>&nbsp;" +
                Utilities::formatMessage(QString::fromUtf8(message.data),32) +
                "</td><td align=\"right\" valign=\"bottom\">");

    if (!day.isEmpty())
        html.append("<div style=\"font-size:18px;color:gray\">"
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

    html.append("<div style=\"font-size:18px;color:gray\">" + time +
                "</div></td></tr></table></td></tr></table>"
                "<div style=\"font-size:5px\">&nbsp;</div>");

    return html;
}


void ChatArea::loadLogMessages(QList<FMessage> messages)
{
    widgetAtBottom = 0;

    bool found = false;
    for (int j = widgetList.length() -1; j >= 0; j--)
    {
        QWidget *widget = (QWidget *)widgetList.at(j);
        if (!widget->visibleRegion().isEmpty())
        {
            if (found)
            {
                widgetAtBottom = widget;
                break;
            }
            else
                found = true;
        }

    }

    // Stop autoscrolling while we load logged messages
    loadingMessages = true;

    for (int i=0; i<messages.length();i++)
    {
        FMessage message = messages.at(i);
        insertMessageAtTop(message);
    }

    QTimer::singleShot(0,this,SLOT(showPrevWidget()));
}

void ChatArea::showPrevWidget()
{
    if (widgetAtBottom)
    {
        qApp->processEvents();
        ensureWidgetVisible(widgetAtBottom,0,0);
        qApp->processEvents();
    }
    loadingMessages = false;
}

void ChatArea::goToBottom()
{
    QScrollBar *bar = verticalScrollBar();

    bar->setValue(bar->maximum());
}

void ChatArea::deleteAllMessages()
{
    foreach(QWidget *widget, widgetList)
    {
        grid->removeWidget(widget);
        delete widget;
    }

    //viewport()->hide();
    widgetList.clear();
}

void ChatArea::requestUpdateTimestamps()
{
    emit updateTimestamps();
}

/*

bool ChatArea::event(QEvent *e)
{
    Utilities::logData("event: " + QString::number(e->type()));

    return QScrollArea::event(e);
}

*/
