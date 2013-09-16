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

#include <QTextDocumentFragment>
#include <QMaemo5InformationBox>
#include <QApplication>
#include <QClipboard>
#include <QInputMethodEvent>
#include <QDateTime>

#include "Whatsapp/util/utilities.h"

#include "chattextedit.h"
#include "client.h"

#include <QX11Info>

#include "Gui/X.h"

#include "hildon-input-method/hildon-im-protocol.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define MAX_LINES       5

ChatTextEdit::ChatTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    QFontMetrics metrics(currentFont());
    fontHeight = metrics.height();

    isEmojiWidgetOpen = false;
    isComposing = false;

    connect(&composingTimer,SIGNAL(timeout()),
            this,SLOT(verifyPaused()));

    connect(this,SIGNAL(textChanged()),
            this,SLOT(textChanged()));

    installEventFilter(this);
}


bool ChatTextEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = (QKeyEvent *) event;

        if (keyEvent->nativeScanCode() == 36)
        {
            // Return pressed
            if (Client::enterIsSend)
            {
                isComposing = false;
                emit returnPressed();
                return true;
            }
        }
        else if (keyEvent->nativeScanCode() == 54 &&
                 keyEvent->modifiers() == Qt::ControlModifier)
        {
            // Copy
            QTextCursor cursor = textCursor();
            if (cursor.hasSelection())
            {
                QTextDocumentFragment selection = cursor.selection();
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(Utilities::htmlToWAText(selection.toHtml()));

                QMaemo5InformationBox::information(this,"Copied");
                return true;
            }
        }
        else if (keyEvent->nativeScanCode() == 55 &&
                 keyEvent->modifiers() == Qt::ControlModifier)
        {
            // Paste event
            QTextCursor cursor = textCursor();
            QClipboard *clipboard = QApplication::clipboard();

            cursor.insertHtml(Utilities::WATextToHtml(clipboard->text(),32,false));

            return true;
        }
        else if (!isComposing)
        {
            isComposing = true;
            emit composing();
        }
        else
        {
            lastKeyPressed = QDateTime::currentMSecsSinceEpoch();
            composingTimer.start(2000);
        }
    }
    else if (event->type() == QEvent::InputMethod)
    {
        QInputMethodEvent *inputEvent = (QInputMethodEvent *) event;

        //Utilities::logData("Commit String: '" + inputEvent->commitString() + "'");
        if (inputEvent->commitString() == "\n" && Client::enterIsSend)
        {
            // Let's hide the keyboard if it was shown
            QTimer::singleShot(0,this,SLOT(closeKB()));
            isComposing = false;
            emit returnPressed();
            return true;
        }
    }

    return QTextEdit::eventFilter(obj,event);
}


static Window findHildonIm()
{
    union
    {
        Window *win;
        unsigned char *val;
    } value;

    Window result = 0;
    ulong n = 0;
    ulong extra = 0;
    int format = 0;
    Atom realType;

    int status = XGetWindowProperty(QX11Info::display(), QX11Info::appRootWindow(),
                    XInternAtom(QX11Info::display(), "_HILDON_IM_WINDOW", False), 0L, 4L, 0,
                    XA_WINDOW, &realType, &format,
                    &n, &extra, (unsigned char **) &value.val);

    if (status == Success && realType == XA_WINDOW
          && format == HILDON_IM_WINDOW_ID_FORMAT && n == 1 && value.win != 0) {
        result = value.win[0];
        XFree(value.val);
    } else {
        qWarning("QHildonInputContext: Unable to get the Hildon IM window id");
    }

    return result;
}

void ChatTextEdit::closeKB()
{
    Window w = findHildonIm();

    if (!w)
        return;

    XEvent ev;
    memset(&ev, 0, sizeof(XEvent));

    HildonIMActivateMessage *msg = reinterpret_cast<HildonIMActivateMessage *>(&ev.xclient.data);

    ev.xclient.type = ClientMessage;
    ev.xclient.window = w;
    ev.xclient.message_type = XInternAtom(QX11Info::display(), "_HILDON_IM_ACTIVATE", False);
    ev.xclient.format = HILDON_IM_ACTIVATE_FORMAT;

    msg->input_window = winId();
    msg->app_window = window()->winId();
    msg->cmd = HILDON_IM_HIDE;
    msg->trigger = HILDON_IM_TRIGGER_NONE;

    XSendEvent(QX11Info::display(),w,false,NoEventMask,&ev);
    XSync(QX11Info::display(),false);
}

void ChatTextEdit::verifyPaused()
{
    if ((QDateTime::currentMSecsSinceEpoch() - lastKeyPressed) > 2000)
    {
        composingTimer.stop();
        if (isComposing)
        {
            isComposing = false;
            emit paused();
        }
    }
}

void ChatTextEdit::textChanged()
{
    int editHeight = size().height();
    double textLines = (editHeight - 39) / fontHeight;

    if (textLines < MAX_LINES)
    {
        int docHeight = document()->size().height();

        QFontMetrics metrics(currentFont());

        fontHeight = metrics.height();

        double docLines = (docHeight - 8) / fontHeight;

        if (abs(docLines) != abs(textLines))
        {
            setFixedHeight((docLines * fontHeight) + 39);
            update();
        }
    }
}

void ChatTextEdit::addEmoji(QString emojiName)
{
    if (isEmojiWidgetOpen)
        closeEmojiWidget();

    QTextCursor cursor = textCursor();

    QString html = "<img src=\"/usr/share/yappari/icons/32x32/" + emojiName +".png\" />";

    cursor.insertHtml(html);
    setFocus();
}

void ChatTextEdit::setMainWindow(QWidget *mainWindow)
{
    this->mainWindow = mainWindow;
}

void ChatTextEdit::selectEmojiButtonClicked()
{
    if (!isEmojiWidgetOpen)
        openEmojiWidget();
    else
        closeEmojiWidget();
}

void ChatTextEdit::openEmojiWidget()
{
    emojiWidget = new SelectEmojiWidget(mainWindow);

    connect(emojiWidget,SIGNAL(emojiSelected(QString)),this,SLOT(addEmoji(QString)));

    isEmojiWidgetOpen = true;

    emojiWidget->show();
}

void ChatTextEdit::closeEmojiWidget()
{
    if (isEmojiWidgetOpen)
    {
        emojiWidget->hide();
        emojiWidget->deleteLater();
        isEmojiWidgetOpen = false;
    }
}
