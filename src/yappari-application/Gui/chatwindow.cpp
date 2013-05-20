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
#include <QMaemo5InformationBox>
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
#include "mediaselectdialog.h"
#include "contactinfowindow.h"
#include "globalconstants.h"

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"

#include "hildon-input-method/hildon-im-protocol.h"

#include "Whatsapp/util/rfc6234/sha224-256.c"

#include "client.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define TEXTEDIT_HEIGHT     70
#define MAX_FILE_SIZE       12582912

ChatWindow::ChatWindow(Contact *contact, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    this->contact = contact;

    ui->setupUi(this);
    ui->scrollArea->init();

    muted = false;
    muteExpireTimestamp = 0;

    connect(ui->scrollArea,SIGNAL(mediaDownload(FMessage)),
            this,SLOT(mediaDownloadRequested(FMessage)));
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendButtonClicked()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),
            ui->textEdit,SLOT(selectEmojiButtonClicked()));

    ui->centralwidget->installEventFilter(this);

    if (contact->type == Contact::TypeContact)
        available(contact->isOnline,contact->lastSeen);

    isPeerComposing = false;
    isMyselfComposing = false;
    // setWindowTitle(Utilities::removeEmoji(contact.name));

    setAttribute(Qt::WA_QuitOnClose,false);

    ui->textEdit->setMainWindow(this);
    ui->textEdit->setFocus();

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
                         (unsigned char *)contact->jid.toUtf8().constData(),
                         contact->jid.toUtf8().length()
                         );
    }

    logger.init(contact->jid);

    connect(this,SIGNAL(logMessage(FMessage)),
            &logger,SLOT(logMessage(FMessage)));

    connect(this,SIGNAL(updateLoggedMessage(FMessage)),
            &logger,SLOT(updateLoggedMessage(FMessage)));

    connect(this,SIGNAL(updateUriMessage(FMessage)),
            &logger,SLOT(updateUriMessage(FMessage)));

    connect(this,SIGNAL(updateDuration(FMessage)),
            &logger,SLOT(updateDuration(FMessage)));

    connect(ui->actionDeleteAll,SIGNAL(triggered()),
            this,SLOT(deleteAllMessages()));

    connect(ui->actionSendMultimedia,SIGNAL(triggered()),
            this,SLOT(sendMultimediaMessage()));

    connect(ui->actionMute,SIGNAL(triggered()),
            this,SLOT(mute()));

    connect(ui->actionViewContact,SIGNAL(triggered()),
            this,SLOT(viewContact()));

    connect(ui->scrollArea,SIGNAL(topReached()),
            this,SLOT(readMoreLogLines()));

    readMoreLogLines();

    qApp->processEvents();
    ui->scrollArea->goToBottom();

    connect(ui->textEdit,SIGNAL(paused()),
            this,SLOT(myselfPaused()));

    connect(ui->textEdit,SIGNAL(composing()),
            this,SLOT(myselfComposing()));

    connect(ui->textEdit,SIGNAL(returnPressed()),
            this,SLOT(sendButtonClicked()));

    Utilities::logData("Created chat window for " + contact->jid + " ID: " +
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

    QString text = Utilities::htmlToWAText(ui->textEdit->toHtml());

    if (!text.isEmpty())
    {
        FMessage message(contact->jid,true);
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

void ChatWindow::setContact(Contact *contact)
{
    this->contact = contact;
    setWindowTitle(Utilities::removeEmoji(contact->name));
}

const Contact& ChatWindow::getContact() const
{
    return *(this->contact);
}

void ChatWindow::messageStatusUpdate(FMessage& message)
{
    ui->scrollArea->updateStatus(message);
    emit updateLoggedMessage(message);
}

void ChatWindow::available(bool online, qint64 lastSeen)
{
    QString text;

    contact->isOnline = online;
    contact->lastSeen = lastSeen;

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
    if (contact->type == Contact::TypeContact)
    {
        isMyselfComposing = true;

        FMessage message(contact->jid,true);
        message.type = FMessage::ComposingMessage;

        emit sendMessage(message);
    }
}

void ChatWindow::myselfPaused()
{
    if (contact->type == Contact::TypeContact)
    {
        isMyselfComposing = false;

        FMessage message(contact->jid,true);
        message.type = FMessage::PausedMessage;

        emit sendMessage(message);
    }
}


void ChatWindow::composing()
{
    isPeerComposing = true;
    ui->typingStatusLabel->setText(Utilities::removeEmoji(contact->name) + " is typing...");
}

void ChatWindow::paused()
{
    if (isPeerComposing)
        ui->typingStatusLabel->setText(Utilities::removeEmoji(contact->name) + " stopped typing.");
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->centralwidget)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            ui->textEdit->closeEmojiWidget();

            return true;
        }

    }

    return QMainWindow::eventFilter(obj,event);
}

void ChatWindow::sendMultimediaMessage()
{
    // Select type of media to send
    MediaSelectDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        // Something I guess

        int waType = dialog.getMediaSelected();

        QString mediaFolder = Utilities::getPathFor(waType, true);
        QString fileExtensions;

        switch (waType)
        {
            case FMessage::Video:
                fileExtensions = EXTENSIONS_VIDEO;
                break;

            case FMessage::Audio:
                fileExtensions = EXTENSIONS_AUDIO;
                break;

            case FMessage::Image:
                fileExtensions = EXTENSIONS_IMAGE;
                break;

            default:
                return;

        }

        QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        mediaFolder,
                                                        fileExtensions);

        if (!fileName.isNull())
        {
            QFile file(fileName);

            if (file.size() > MAX_FILE_SIZE)
            {
                QMaemo5InformationBox::information(this,"You can't send files bigger than 12 MB",
                                                   QMaemo5InformationBox::NoTimeout);
            }
            else
            {
                // Confirmation dialog
                QMessageBox msg(this);
                int index = fileName.lastIndexOf('/');
                index++;
                msg.setText("Are you sure you want to send the file " +
                            fileName.mid(index) + "?");
                msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

                if (msg.exec() == QMessageBox::Yes)
                {
                    Utilities::logData("File selected: " + fileName);

                    FMessage msg(JID_DOMAIN, true);

                    msg.status = FMessage::Unsent;
                    msg.type = FMessage::RequestMediaMessage;
                    msg.media_size = file.size();
                    msg.remote_resource = contact->jid;
                    msg.media_wa_type = waType;
                    msg.media_name = fileName;

                    // We still don't know the duration in seconds
                    msg.media_duration_seconds = 0;

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

    connect(mediaUpload,SIGNAL(sslError(MediaUpload*)),
            this,SLOT(sslErrorHandler(MediaUpload*)));

    connect(mediaUpload,SIGNAL(httpError(MediaUpload*)),
            this,SLOT(httpErrorHandler(MediaUpload*)));

    connect(mediaUpload,SIGNAL(headersReceived(qint64)),
            this,SLOT(increaseDownloadCounter(qint64)));

    /*
    connect(mediaUpload,SIGNAL(requestSent(qint64)),
            this,SLOT(increaseUploadCounter(qint64)));
    */

    mediaUpload->sendMedia(contact->jid,msg);

}


void ChatWindow::mediaUploadStarted(MediaUpload *mediaUpload, FMessage msg)
{
    emit logMessage(msg);

    showMessageInUI(msg);

    connect(mediaUpload,SIGNAL(progress(FMessage,float)),
            ui->scrollArea,SLOT(updateProgress(FMessage,float)));
}

void ChatWindow::mediaUploadFinished(MediaUpload *mediaUpload, FMessage msg)
{
    disconnect(mediaUpload, 0, 0, 0);
    mediaUpload->deleteLater();

    // Update duration
    ui->scrollArea->updateImage(msg);

    emit updateDuration(msg);

    emit sendMessage(msg);

    // Increase counters
    int type;
    switch (msg.media_wa_type)
    {
        case FMessage::Image:
            type = DataCounters::ImageBytes;
            break;

        case FMessage::Audio:
            type = DataCounters::AudioBytes;
            break;

        case FMessage::Video:
            type = DataCounters::VideoBytes;
            break;

        default:
            type = DataCounters::ProtocolBytes;
            break;
    }

    Client::dataCounters.increaseCounter(type, 0, msg.media_size);
}

void ChatWindow::sslErrorHandler(MediaUpload *mediaUpload)
{
    disconnect(mediaUpload, 0, 0, 0);
    mediaUpload->deleteLater();

    Utilities::logData("SSL error while trying to upload a picture. Maybe bad time & date settings on the phone?");

    QMaemo5InformationBox::information(this,"There has been an SSL error while trying to upload a multimedia message.\nPlease check your phone has the correct date & time settings.",
                                       QMaemo5InformationBox::NoTimeout);
}

void ChatWindow::httpErrorHandler(MediaUpload *mediaUpload)
{
    disconnect(mediaUpload, 0, 0, 0);
    mediaUpload->deleteLater();

    Utilities::logData("HTTP error while trying to upload a picture.");

    QMaemo5InformationBox::information(this,"There has been an HTTP error while trying to upload a multimedia message.\nPlease check your log for more details.",
                                       QMaemo5InformationBox::NoTimeout);
}

void ChatWindow::mediaDownloadRequested(FMessage msg)
{
    Utilities::logData("Media download requested: " + msg.media_url);

    MediaDownload *mediaDownload = new MediaDownload();

    connect(mediaDownload,SIGNAL(progress(FMessage,float)),
            ui->scrollArea,SLOT(updateProgress(FMessage,float)));

    connect(mediaDownload,SIGNAL(downloadFinished(MediaDownload *, FMessage)),
            this,SLOT(mediaDownloadFinished(MediaDownload *, FMessage)));

    connect(mediaDownload,SIGNAL(httpError(MediaDownload*, FMessage, int)),
            this,SLOT(mediaDownloadError(MediaDownload*, FMessage, int)));

    connect(mediaDownload,SIGNAL(headersReceived(qint64)),
            this,SLOT(increaseDownloadCounter(qint64)));

    connect(mediaDownload,SIGNAL(requestSent(qint64)),
            this,SLOT(increaseUploadCounter(qint64)));

    mediaDownload->backgroundTransfer(msg);
}

void ChatWindow::mediaDownloadFinished(MediaDownload *mediaDownload, FMessage msg)
{
    disconnect(mediaDownload, 0, 0, 0);
    mediaDownload->deleteLater();

    Utilities::logData("About to update the URI");

    ui->scrollArea->updateUri(msg);

    emit updateUriMessage(msg);

    // Increase counters
    int type;
    switch (msg.media_wa_type)
    {
        case FMessage::Image:
            type = DataCounters::ImageBytes;
            break;

        case FMessage::Audio:
            type = DataCounters::AudioBytes;
            break;

        case FMessage::Video:
            type = DataCounters::VideoBytes;
            break;

        default:
            type = DataCounters::ProtocolBytes;
            break;
    }

    Client::dataCounters.increaseCounter(type, msg.media_size, 0);
}

void ChatWindow::mediaDownloadError(MediaDownload *mediaDownload, FMessage msg, int errorCode)
{
    disconnect(mediaDownload, 0, 0, 0);
    mediaDownload->deleteLater();

    Utilities::logData("There has been an error while trying to download media. Maybe the file is not available in the servers anymore.  Error = " +
                       QString::number(errorCode));

    QMaemo5InformationBox::information(this,"There has been an error while trying to download media.\nMaybe the file is not available in the servers anymore.",
                                       QMaemo5InformationBox::NoTimeout);

    ui->scrollArea->resetButton(msg);
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
            emit mute(contact->jid,muted,muteExpireTimestamp);
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
    emit mute(contact->jid,muted,muteExpireTimestamp);
}

FMessage ChatWindow::lastMessage()
{
    return logger.lastMessage();
}

void ChatWindow::updateTimestamps()
{
    Utilities::logData("ChatWindow " + contact->jid + " updateTimeStamps()");
    ui->scrollArea->requestUpdateTimestamps();
}

void ChatWindow::increaseUploadCounter(qint64 bytes)
{
    Client::dataCounters.increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

void ChatWindow::increaseDownloadCounter(qint64 bytes)
{
    Client::dataCounters.increaseCounter(DataCounters::ProtocolBytes, bytes, 0);
}

void ChatWindow::requestPhotoRefresh(QString jid, QString photoId, bool largeFormat)
{
    emit photoRefresh(jid, photoId, largeFormat);
}

void ChatWindow::viewContact()
{
    ContactInfoWindow *window = new ContactInfoWindow(contact,this);

    connect(window,SIGNAL(photoRefresh(QString,QString,bool)),
            this,SLOT(requestPhotoRefresh(QString,QString,bool)));

    emit requestStatus(contact->jid);

    window->setAttribute(Qt::WA_Maemo5StackedWindow);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setWindowFlags(window->windowFlags() | Qt::Window);
    window->show();
}

void ChatWindow::photoReceivedHandler(QImage photo, QString photoId)
{
    emit photoReceived(photo, photoId);
}

void ChatWindow::statusChanged(QString status)
{
    contact->status = status;
    emit userStatusChanged();
}
