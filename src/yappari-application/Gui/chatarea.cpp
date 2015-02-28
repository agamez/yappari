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

#include <QMaemo5InformationBox>
#include <QMaemo5Style>
#include <QApplication>
#include <QClipboard>
#include <QMouseEvent>
#include <QTimer>
#include <QMenu>

#include "globalconstants.h"
#include "chatimageitem.h"
#include "chattextitem.h"
#include "client.h"
#include "chatarea.h"

#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/util/utilities.h"

#include "Dbus/dbusnokiaossobrowserif.h"

#include "globalconstants.h"

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

    connect(this,SIGNAL(contextMenuRequested(QPoint, QObject *)),
            this,SLOT(contextMenu(QPoint, QObject*)));

    loadingMessages = false;
}

void ChatArea::sliderMoved(int value)
{
    if (value == 0)
        emit topReached();
    else if (value == verticalScrollBar()->maximum()) {
        emit bottomReached();
        Utilities::logData("Bottom reached");
    }
}


void ChatArea::sliderRangeChanged(int min, int max)
{
    Q_UNUSED(min);

    if (!loadingMessages && atBottom)
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
    atBottom = (verticalScrollBar()->value() == verticalScrollBar()->maximum());

    if (message.type == FMessage::BodyMessage)
        insertBodyMessage(message,atTop);
    if (message.type == FMessage::MediaMessage)
        insertMediaMessage(message,atTop);
}

void ChatArea::insertWidget(Key key, bool atTop, QWidget *label)
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

    widgets.insert(key, label);
}

void ChatArea::insertBodyMessage(FMessage message, bool atTop)
{
    ChatTextItem *textItem = new ChatTextItem(message,container);
    textItem->installEventFilter(this);
    textItem->setObjectName("ChatTextItem");
    connect(this,SIGNAL(updateTimestamps()),textItem,SLOT(updateTimestamp()));

    insertWidget(message.key, atTop, textItem);
}

void ChatArea::insertMediaMessage(FMessage message, bool atTop)
{

    if (message.media_wa_type == FMessage::Image ||
        message.media_wa_type == FMessage::Video ||
        message.media_wa_type == FMessage::Audio ||
        message.media_wa_type == FMessage::Location)
    {
        ChatImageItem *imageItem = new ChatImageItem(message,container);
        imageItem->installEventFilter(this);
        imageItem->setObjectName("ChatImageItem");

        connect(this,SIGNAL(updateTimestamps()),imageItem,SLOT(updateTimestamp()));
        connect(imageItem,SIGNAL(mediaDownload(FMessage)),
                this,SLOT(mediaDownloadHandler(FMessage)));
        connect(imageItem,SIGNAL(mediaUpload(FMessage)),
                this,SLOT(mediaUploadHandler(FMessage)));

        if (message.live)
            connect(imageItem,SIGNAL(voiceNotePlayed(FMessage)),
                    this,SLOT(voiceNotePlayedHandler(FMessage)));

        insertWidget(message.key, atTop, imageItem);
    }
}

void ChatArea::mediaDownloadHandler(FMessage message)
{
    emit mediaDownload(message);
}

void ChatArea::mediaUploadHandler(FMessage message)
{
    emit mediaUpload(message);
}

void ChatArea::voiceNotePlayedHandler(FMessage message)
{
    emit voiceNotePlayed(message);
}

void ChatArea::updateStatus(FMessage message)
{
    if (widgets.contains(message.key))
    {
        if (message.type == FMessage::BodyMessage)
        {
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

bool ChatArea::isMessageInUI(FMessage msg)
{
    return (widgets.contains(msg.key));
}

/*

bool ChatArea::event(QEvent *e)
{
    Utilities::logData("event: " + QString::number(e->type()));

    return QScrollArea::event(e);
}

*/
bool ChatArea::eventFilter(QObject *obj, QEvent *event)
{
    if (obj != container && event->type() == QEvent::ContextMenu)
    {
        if (obj->objectName() == "ChatTextItem" || obj->objectName() == "ChatImageItem")
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
            emit contextMenuRequested(mouseEvent->globalPos(), obj);
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}


void ChatArea::contextMenu(QPoint p, QObject *obj)
{
    Utilities::logData("Context Menu requested");

    QMenu *menu = new QMenu(this);
    QAction *copy = new QAction("Copy",this);
    QAction *forward = new QAction("Forward", this);
    QAction *deletemsg = new QAction("Delete", this);
    QAction *browser = new QAction("Open in browser", this);

    if (obj->objectName() == "ChatTextItem")
        menu->addAction(copy);

    menu->addAction(forward);
    menu->addAction(deletemsg);

    if (obj->objectName() == "ChatImageItem")
    {
        ChatImageItem *item = (ChatImageItem *) obj;
        FMessage msg = item->getMessage();

        if (!msg.local_file_uri.isEmpty())
            menu->addAction(browser);
    }

    QAction *action = menu->exec(p);
    if (action == copy)
    {
        QClipboard *clipboard = QApplication::clipboard();
        ChatTextItem *item = (ChatTextItem *) obj;
        FMessage msg = item->getMessage();
        clipboard->setText(QString::fromUtf8(msg.data.constData()));

        QMaemo5InformationBox::information(this,"Copied");
    }
    else if (action == forward)
    {
        FMessage msg;
        if (obj->objectName() == "ChatTextItem")
        {
            ChatTextItem *item = (ChatTextItem *) obj;
            msg = item->getMessage();
        }
        else if (obj->objectName() == "ChatImageItem")
        {
            ChatImageItem *item = (ChatImageItem *) obj;
            msg = item->getMessage();
        }

        emit forwardMessage(msg);
    }
    else if (action == browser)
    {
        ChatImageItem *item = (ChatImageItem *) obj;
        FMessage msg = item->getMessage();

        QDBusConnection dbus = QDBusConnection::sessionBus();

        DBusNokiaOssoBrowserIf *browserBus =
        new DBusNokiaOssoBrowserIf(NOKIA_OSSO_BROWSER_DBUS_NAME,
                                   NOKIA_OSSO_BROWSER_DBUS_PATH,
                                   dbus,this);

        browserBus->load_url("file://" + msg.local_file_uri);
    }
    else if (action == deletemsg)
    {
        FMessage msg;
        if (obj->objectName() == "ChatTextItem")
        {
            ChatTextItem *item = (ChatTextItem *) obj;
            msg = item->getMessage();
            grid->removeWidget(item);
            delete item;
        }
        else if (obj->objectName() == "ChatImageItem")
        {
            ChatImageItem *item = (ChatImageItem *) obj;
            msg = item->getMessage();
            grid->removeWidget(item);
            delete item;
        }
        emit deleteMessage(msg);
    }
    Utilities::logData("Exit Menu");
}
