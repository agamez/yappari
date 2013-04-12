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

#include <QAbstractTextDocumentLayout>
#include <QCryptographicHash>
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDateTime>
#include <QRegExp>
#include <QX11Info>

#include <QTimer>
#include <QInputMethodEvent>

#include "Gui/X.h"

#include "selectemojiwidget.h"
#include "chatwindow.h"
#include "ui_chatwindow.h"
#include "conversationdelegate.h"
#include "mutedialog.h"
#include "globalconstants.h"

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"

#include "hildon-input-method/hildon-im-protocol.h"

#include "Whatsapp/util/rfc6234/sha224-256.c"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define TEXTEDIT_HEIGHT     70
#define MAX_FILE_SIZE       12582912

ChatWindow::ChatWindow(Contact contact, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    ui->setupUi(this);
    ui->scrollArea->init();

    QFontMetrics metrics(ui->textEdit->currentFont());
    fontHeight = metrics.height();

    muted = false;
    muteExpireTimestamp = 0;

    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendButtonClicked()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),this,SLOT(selectEmojiButtonClicked()));

    ui->centralwidget->installEventFilter(this);

    if (contact.type == Contact::TypeContact)
        available(contact.isOnline,contact.lastSeen);

    isPeerComposing = false;
    isMyselfComposing = false;
    isEmojiWidgetOpen = false;
    setWindowTitle(contact.name);
    //ui->conversationWidget->setItemDelegate(new ConversationDelegate(ui->conversationWidget));

    setAttribute(Qt::WA_QuitOnClose,false);

    ui->textEdit->setFocus();
    ui->textEdit->installEventFilter(this);

    // Set the thread name
    Atom atom = XInternAtom(QX11Info::display(), "_HILDON_NOTIFICATION_THREAD", False);
    if (atom)
    {
        XChangeProperty (QX11Info::display(),
                         winId(),
                         atom,
                         XA_STRING,
                         8,
                         PropModeReplace,
                         (unsigned char *)contact.jid.toUtf8().constData(),
                         contact.jid.toUtf8().length()
                         );
    }

    connect(&composingTimer,SIGNAL(timeout()),
            this,SLOT(verifyPaused()));

    logger.init(contact.jid);

    connect(this,SIGNAL(logMessage(FMessage)),
            &logger,SLOT(logMessage(FMessage)));

    connect(this,SIGNAL(updateLoggedMessage(FMessage)),
            &logger,SLOT(updateLoggedMessage(FMessage)));

    //connect(ui->actionDeleteAll,SIGNAL(triggered()),
    //        &logger,SLOT(deleteAllMessages()));

    connect(ui->actionDeleteAll,SIGNAL(triggered()),
            this,SLOT(deleteAllMessages()));

    connect(ui->actionSendMultimedia,SIGNAL(triggered()),
            this,SLOT(sendMultimediaMessage()));

    connect(ui->actionMute,SIGNAL(triggered()),
            this,SLOT(mute()));

    connect(ui->scrollArea,SIGNAL(topReached()),
            this,SLOT(readMoreLogLines()));

    readMoreLogLines();

    qApp->processEvents();
    ui->scrollArea->goToBottom();

    connect(ui->textEdit,SIGNAL(textChanged()),
            this,SLOT(textChanged()));

    Utilities::logData("Created chat window for " + contact.jid + " ID: " +
                       QString::number(this->winId()));
}

ChatWindow::~ChatWindow()
{
    delete ui;
}

void ChatWindow::readMoreLogLines()
{
    QList<FMessage> list = logger.lastMessages();

    ui->scrollArea->loadLogMessages(list);
}

void ChatWindow::messageReceived(FMessage& message)
{
    showMessageInUI(message);
    emit logMessage(message);
    isPeerComposing = false;
    ui->typingStatusLabel->clear();
}

void ChatWindow::sendButtonClicked()
{
    // This has to be changed to toHtml() to get smileys
    // eventually.
    // QString text = ui->textEdit->toPlainText();

    QString text = getText();

    if (!text.isEmpty())
    {
        FMessage message(contact.jid,true);
        message.type = FMessage::BodyMessage;
        message.setData(text);

        if (isMyselfComposing)
            myselfPaused();
        emit sendMessage(message);
        emit logMessage(message);

        showMessageInUI(message);
        ui->textEdit->clear();
        ui->textEdit->setFixedHeight(TEXTEDIT_HEIGHT);
    }

    ui->textEdit->setFocus();
}

void ChatWindow::showMessageInUI(FMessage& message)
{
    ui->scrollArea->insertMessageAtBottom(message);
}

void ChatWindow::setContact(Contact& contact)
{
    this->contact = contact;
    setWindowTitle(contact.name);
}

const Contact& ChatWindow::getContact() const
{
    return this->contact;
}

void ChatWindow::messageStatusUpdate(FMessage& message)
{
    ui->scrollArea->updateStatus(message);
    emit updateLoggedMessage(message);
}

void ChatWindow::available(bool online, qint64 lastSeen)
{
    QString text;

    contact.isOnline = online;
    contact.lastSeen = lastSeen;

    if (!online)
    {
        if (!lastSeen)
            text = "Offline";
        else
            text = "Last seen " + DateTimeUtilities::formattedStamp(lastSeen);
    }
    else
        text = "Online";

    setOnlineText(text);
}

void ChatWindow::setOnlineText(QString text)
{
    ui->onlineStatusLabel->setText(text);
}

void ChatWindow::myselfComposing()
{
    if (contact.type == Contact::TypeContact)
    {
        isMyselfComposing = true;

        FMessage message(contact.jid,true);
        message.type = FMessage::ComposingMessage;

        emit sendMessage(message);

        lastKeyPressed = QDateTime::currentMSecsSinceEpoch();
        composingTimer.start(2000);
    }
}

void ChatWindow::myselfPaused()
{
    if (contact.type == Contact::TypeContact)
    {
        composingTimer.stop();

        isMyselfComposing = false;

        FMessage message(contact.jid,true);
        message.type = FMessage::PausedMessage;

        emit sendMessage(message);
    }
}


void ChatWindow::composing()
{
    isPeerComposing = true;
    ui->typingStatusLabel->setText(contact.name + " is typing...");
}

void ChatWindow::paused()
{
    if (isPeerComposing)
        ui->typingStatusLabel->setText(contact.name + " stopped typing.");
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->textEdit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent *) event;

            if (keyEvent->nativeScanCode() == 36)
            {
                QTimer::singleShot(0,this,SLOT(sendButtonClicked()));
                return true;
            }
            else if (!isMyselfComposing)
                QTimer::singleShot(0,this,SLOT(myselfComposing()));
            else
                lastKeyPressed = QDateTime::currentMSecsSinceEpoch();
        }
        else if (event->type() == QEvent::InputMethod)
        {
            QInputMethodEvent *inputEvent = (QInputMethodEvent *) event;

            //Utilities::logData("Commit String: '" + inputEvent->commitString() + "'");
            if (inputEvent->commitString() == "\n")
            {
                // Let's hide the keyboard if it was shown
                QTimer::singleShot(0,this,SLOT(closeKB()));
                QTimer::singleShot(0,this,SLOT(sendButtonClicked()));
                return true;
            }
        }
    }
    else if (obj == ui->centralwidget)
    {
        if (event->type() == QEvent::MouseButtonPress && isEmojiWidgetOpen)
        {
            closeEmojiWidget();

            return true;
        }

    }

    return QMainWindow::eventFilter(obj,event);
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

void ChatWindow::closeKB()
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
    msg->app_window = ui->textEdit->window()->winId();
    msg->cmd = HILDON_IM_HIDE;
    msg->trigger = HILDON_IM_TRIGGER_NONE;

    XSendEvent(QX11Info::display(),w,false,NoEventMask,&ev);
    XSync(QX11Info::display(),false);
}

void ChatWindow::verifyPaused()
{
    if ((QDateTime::currentMSecsSinceEpoch() - lastKeyPressed) > 2000)
        QTimer::singleShot(0,this,SLOT(myselfPaused()));
}

void ChatWindow::textChanged()
{
    int docHeight = ui->textEdit->document()->size().height();
    int editHeight = ui->textEdit->size().height();

    QFontMetrics metrics(ui->textEdit->currentFont());

    fontHeight = metrics.height();

    double docLines = (docHeight - 8) / fontHeight;
    double textLines = (editHeight - 39) / fontHeight;

    if (abs(docLines) != abs(textLines))
    {
        ui->textEdit->setFixedHeight((docLines * fontHeight) + 39);
        ui->textEdit->update();
    }
}

void ChatWindow::selectEmojiButtonClicked()
{
    if (!isEmojiWidgetOpen)
        openEmojiWidget();
    else
        closeEmojiWidget();
}

void ChatWindow::openEmojiWidget()
{
    emojiWidget = new SelectEmojiWidget(this);

    connect(emojiWidget,SIGNAL(emojiSelected(QString)),this,SLOT(addEmoji(QString)));

    isEmojiWidgetOpen = true;

    emojiWidget->show();
}

void ChatWindow::closeEmojiWidget()
{
    emojiWidget->hide();
    emojiWidget->deleteLater();
    isEmojiWidgetOpen = false;
}

void ChatWindow::addEmoji(QString emojiName)
{
    closeEmojiWidget();

    QTextCursor cursor = ui->textEdit->textCursor();

    QString html = "<img src=\"/usr/share/yappari/icons/32x32/" + emojiName +".png\" />";

    cursor.insertHtml(html);
    ui->textEdit->setFocus();
}

QString ChatWindow::getText()
{
    QString html = ui->textEdit->toHtml();

    QRegExp htmlreg("(<img src=\"[^\"]+\" />)");

    int pos = 0;
    while ((pos = htmlreg.indexIn(html,pos)) != -1)
    {
        QString image = htmlreg.cap(1);

        QRegExp imagereg("([a-f0-9]+)-([a-f0-9]+).png");

        int imagePos = imagereg.indexIn(image);
        if (imagePos != -1)
        {
            QByteArray array;
            uchar e1 = imagereg.cap(1).toInt(0,16);
            if (e1 == 0x9F)
            {
                quint16 e2 = imagereg.cap(2).toInt(0,16);
                array.append(0xF0);
                array.append(e1);
                array.append((e2 & 0xFF00) >> 8);
                array.append(e2 & 0xFF);
            }
            else if (e1 == 0xE2 || e1 == 0xE3)
            {
                quint16 e2 = imagereg.cap(2).toInt(0,16);
                array.append(e1);
                array.append((e2 & 0xFF00) >> 8);
                array.append(e2 & 0xFF);
            }
            else
            {
                uchar e2 = imagereg.cap(2).toInt() + 0x80;

                array.append(0xEE);
                array.append(e1);
                array.append(e2);
            }

            html.replace(pos,image.length(),QString::fromUtf8(array.constData()));
        }
    }

    QTextDocument doc;
    doc.setHtml(html);

    return doc.toPlainText();
}

void ChatWindow::sendMultimediaMessage()
{
    QDir home = QDir::home();
    QString imagesFolder = home.path() + IMAGES_DIR;
    if (!home.exists(imagesFolder))
        imagesFolder = (!home.exists(home.path() + DEFAULT_DIR)) ?
                    home.path() : home.path() + DEFAULT_DIR;

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"),
                                                    imagesFolder,
                                                    tr("Images (*.png *.jpg *.jpeg *.gif *.PNG *.JPG *.JPEG *.GIF)"));

    if (!fileName.isNull())
    {
        QFile file(fileName);

        if (file.size() > MAX_FILE_SIZE)
        {
            QMessageBox msg(this);

            msg.setText("You can't send files bigger than 12 MB");
            msg.exec();
        }
        else
        {
            Utilities::logData("File selected: " + fileName);

            FMessage msg(JID_DOMAIN, true);

            msg.status = FMessage::Unsent;
            msg.type = FMessage::RequestMediaMessage;
            msg.media_size = file.size();
            msg.remote_resource = contact.jid;
            msg.media_wa_type = FMessage::Image;
            msg.media_name = fileName;

            if (file.open(QIODevice::ReadOnly))
            {
                QByteArray bytes = file.readAll();
                SHA256Context sha256;

                SHA256Reset(&sha256);
                SHA256Input(&sha256, reinterpret_cast<uint8_t *>(bytes.data()),
                            bytes.length());
                QByteArray result;
                result.resize(SHA256HashSize);
                SHA256Result(&sha256, reinterpret_cast<uint8_t *>(result.data()));
                msg.setData(result.toBase64());

                file.close();

                emit sendMessage(msg);
            }
        }
    }
}

void ChatWindow::mediaUploadAccepted(FMessage msg)
{

    Utilities::logData("Media upload accepted: " + msg.media_url);

    MediaUpload *mediaUpload = new MediaUpload();

    connect(mediaUpload,SIGNAL(sendMessage(MediaUpload*,FMessage)),
            this,SLOT(mediaUploadFinished(MediaUpload*, FMessage)));

    connect(mediaUpload,SIGNAL(readyToSendMessage(MediaUpload*,FMessage)),
            this,SLOT(mediaUploadStarted(MediaUpload*, FMessage)));

    mediaUpload->sendPicture(contact.jid, msg.media_name, msg.media_url);
}


void ChatWindow::mediaUploadStarted(MediaUpload *mediaUpload, FMessage msg)
{
    Q_UNUSED(mediaUpload);

    emit logMessage(msg);

    showMessageInUI(msg);
}

void ChatWindow::mediaUploadFinished(MediaUpload *mediaUpload, FMessage msg)
{
    disconnect(mediaUpload, 0, 0, 0);
    mediaUpload->deleteLater();

    emit sendMessage(msg);
}

void ChatWindow::deleteAllMessages()
{
    QMessageBox msg(this);

    msg.setText("Are you sure you want to clear the conversation history?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    if (msg.exec() == QMessageBox::Yes)
    {
        logger.deleteAllMessages();
        ui->scrollArea->deleteAllMessages();
    }
}

void ChatWindow::setMute(qint64 timestamp)
{
    muted = true;
    muteExpireTimestamp = timestamp;
    ui->actionMute->setText("Unmute");
}

void ChatWindow::mute()
{
    if (!muted)
    {
        MuteDialog dialog(this);

        if (dialog.exec() == QDialog::Accepted)
        {
            muted = true;
            muteExpireTimestamp = dialog.getMuteExpireTimestamp();
            //Utilities::logData("Subject change to: " + newSubject);
            //emit changeSubject(contact.jid,newSubject);

            ui->actionMute->setText("Unmute");
            emit mute(contact.jid,muted,muteExpireTimestamp);
        }
    }
    else
        unmute();
}

void ChatWindow::unmute()
{
    muted = false;
    muteExpireTimestamp = 0;
    ui->actionMute->setText("Mute");
    emit mute(contact.jid,muted,muteExpireTimestamp);
}

FMessage ChatWindow::lastMessage()
{
    return logger.lastMessage();
}

void ChatWindow::updateTimestamps()
{
    Utilities::logData("ChatWindow " + contact.jid + " updateTimeStamps()");
    ui->scrollArea->requestUpdateTimestamps();
}



