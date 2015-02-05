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

#include <QAbstractTextDocumentLayout>
#include <QMaemo5InformationBox>
#include <QCryptographicHash>
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QFileDialog>
#include <QInputDialog>
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
#include "imagepreviewdialog.h"
#include "videopreviewdialog.h"
#include "audiopreviewdialog.h"
#include "contactinfowindow.h"
#include "globalconstants.h"

#include "Multimedia/audiorecorder.h"

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/ioexception.h"

#include "hildon-input-method/hildon-im-protocol.h"

#include "Whatsapp/util/rfc6234/sha224-256.c"

#include "client.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>

// Solve clash between Xlib FocusIn and QEvent::FocusIn
enum { XFocusIn = FocusIn };
#undef FocusIn

#define TEXTEDIT_HEIGHT     70
#define MAX_FILE_SIZE       16777216

ChatWindow::ChatWindow(Contact *contact, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindow)
{
    this->contact = contact;

    ui->setupUi(this);
    ui->scrollArea->init();

    ui->recordingLabel->hide();
    // ui->timeLabel->hide();

    muted = false;
    muteExpireTimestamp = 0;

    connect(ui->scrollArea,SIGNAL(mediaDownload(FMessage)),
            this,SLOT(mediaDownloadRequested(FMessage)));
    connect(ui->scrollArea,SIGNAL(mediaUpload(FMessage)),
            this,SLOT(mediaUploadAccepted(FMessage)));
    connect(ui->scrollArea,SIGNAL(voiceNotePlayed(FMessage)),
            this,SLOT(sendVoiceNotePlayed(FMessage)));
    connect(ui->scrollArea,SIGNAL(forwardMessage(FMessage)),
            this,SLOT(forwardMessageRequested(FMessage)));
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(sendButtonClicked()));
    connect(ui->sendButton,SIGNAL(pressed()),this,SLOT(startRecording()));
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
            this,SLOT(selectMultimediaMessage()));

    connect(ui->actionMute,SIGNAL(triggered()),
            this,SLOT(mute()));

    connect(ui->actionViewContact,SIGNAL(triggered()),
            this,SLOT(viewContact()));

    connect(ui->actionBlock,SIGNAL(triggered()),
            this,SLOT(blockOrUnblock()));

    connect(ui->scrollArea,SIGNAL(topReached()),
            this,SLOT(readMoreLogLines()));

    connect(ui->scrollArea,SIGNAL(bottomReached()),
            this,SLOT(setMessagesAsRead()));

    setBlock(contact->blocked);

    readMoreLogLines();

    qApp->processEvents();
    ui->scrollArea->goToBottom();

    connect(ui->textEdit,SIGNAL(paused()),
            this,SLOT(myselfPaused()));

    connect(ui->textEdit,SIGNAL(composing()),
            this,SLOT(myselfComposing()));

    connect(ui->textEdit,SIGNAL(returnPressed()),
            this,SLOT(sendButtonClicked()));

    connect(ui->textEdit,SIGNAL(textChanged()),
            this,SLOT(textChanged()));

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

void ChatWindow::setMessagesAsRead()
{
    if(!isActiveWindow()) return;

    FMessage msg = logger.lastMessage();
    if(msg.key.from_me || msg.status == FMessage::ReceivedByTarget) return;

    if(msg.key.remote_jid.right(5) != "@g.us" && !Client::blueChecks) return;

    emit messageRead(msg);
    msg.status = FMessage::ReceivedByTarget;
    logger.updateLoggedMessage(msg);
    Utilities::logData("Message should now be marked as read");
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
    if (!ui->textEdit->document()->isEmpty())
    {
        QString text = Utilities::htmlToWAText(ui->textEdit->toHtml());

        FMessage message(contact->jid,true);
        message.type = FMessage::BodyMessage;
        message.setData(text);

        if (isMyselfComposing)
            myselfPaused();

        sendTextMessage(message);

        ui->textEdit->clear();
        ui->textEdit->setFixedHeight(TEXTEDIT_HEIGHT);
    }

    ui->textEdit->setFocus();
}

void ChatWindow::sendTextMessage(FMessage& message)
{
    emit sendMessage(message);
    emit logMessage(message);

    showMessageInUI(message);
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

void ChatWindow::textChanged()
{
    if (!ui->textEdit->document()->isEmpty())
    {
        ui->sendButton->setText("Send");
        ui->sendButton->setIcon(QIcon());
    }
    else
    {
        ui->sendButton->setText("");
        ui->sendButton->setIcon(QIcon("/usr/share/yappari/icons/48x48/voice_overlay_icon.png"));
    }
}

void ChatWindow::myselfComposing(int waType)
{
    //if (contact->type == Contact::TypeContact) Let's keep it for future broadcast implementation
    //{
        isMyselfComposing = true;

        FMessage message(contact->jid,true);
        message.type = FMessage::ComposingMessage;
        message.media_wa_type = waType;

        emit sendMessage(message);
    //}
}

void ChatWindow::myselfPaused()
{
    //if (contact->type == Contact::TypeContact) Let's keep it for future broadcast implementation
    //{
        isMyselfComposing = false;

        FMessage message(contact->jid,true);
        message.type = FMessage::PausedMessage;

        emit sendMessage(message);
    //}
}


void ChatWindow::composing(QString participant, QString media)
{
    isPeerComposing = true;
    QString text = (media == "audio") ? " is recording audio..." : " is typing...";
    QString n = participant=="" ? contact->name : participant;
    ui->typingStatusLabel->setText(Utilities::removeEmoji(n) + text);
}

void ChatWindow::paused(QString participant)
{
    //Participan not used yet, but can be used in group when more than one is typing
    if (isPeerComposing)
        // ui->typingStatusLabel->setText(Utilities::removeEmoji(contact->name) + " stopped typing.");
        ui->typingStatusLabel->clear();
}

bool ChatWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::WindowActivate)
    {
        setMessagesAsRead();
        return true;
    }

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

void ChatWindow::selectMultimediaMessage()
{
    // Select type of media to send
    MediaSelectDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        // Something I guess

        int waType = dialog.getMediaSelected();

        QString mediaFolder;
        QString fileExtensions;

        switch (waType)
        {
            case FMessage::Video:
                fileExtensions = EXTENSIONS_VIDEO;
                mediaFolder = Client::lastVideoDir;
                break;

            case FMessage::Audio:
                fileExtensions = EXTENSIONS_AUDIO;
                mediaFolder = Client::lastAudioDir;
                break;

            case FMessage::Image:
                fileExtensions = EXTENSIONS_IMAGE;
                mediaFolder = Client::lastImageDir;
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

            emit updateLastDir(waType, fileName.left(fileName.lastIndexOf("/")));

            if (file.size() > MAX_FILE_SIZE)
            {
                QMaemo5InformationBox::information(this,"You can't send files bigger than 16 MB",
                                                   QMaemo5InformationBox::NoTimeout);
            }
            else
            {
                Utilities::logData("File selected: " + fileName);

                QString short_caption;
                if(waType==FMessage::Video)
                {
                    VideoPreviewDialog preview(this, fileName);
                    if (preview.exec() != QDialog::Accepted) {
                        return;
                    }
                    short_caption = preview.getCaption().left(160);
                }
                else if(waType==FMessage::Audio)
                {
                    AudioPreviewDialog preview(this, fileName);
                    if (preview.exec() != QDialog::Accepted) {
                        return;
                    }
                    short_caption = preview.getCaption().left(160);
                }
                else if(waType==FMessage::Image)
                {
                    ImagePreviewDialog preview(this, fileName);
                    if (preview.exec() != QDialog::Accepted) {
                        return;
                    }
                    short_caption = preview.getCaption().left(160);
                }

                Utilities::logData("Caption shortened to " + short_caption);
                sendMultimediaMessage(fileName, waType, false, short_caption);
            }
        }
    }
}

void ChatWindow::sendMultimediaMessage(QString fileName, int waType, bool live, QString caption)
{
    QFile file(fileName);

    FMessage msg(JID_DOMAIN, true);

    msg.status = FMessage::Unsent;
    msg.type = FMessage::RequestMediaMessage;
    msg.media_size = file.size();
    msg.remote_resource = contact->jid;
    msg.media_wa_type = waType;
    msg.local_file_uri = fileName;
    msg.live = live;
    msg.media_caption = caption;

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

    connect(mediaUpload,SIGNAL(httpError(MediaUpload*,QString)),
            this,SLOT(httpErrorHandler(MediaUpload*,QString)));

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
    if (!ui->scrollArea->isMessageInUI(msg))
    {
        Utilities::logData("Message is not in the UI");

        emit logMessage(msg);

        showMessageInUI(msg);
    }

    Utilities::logData("Message is live: "  + QString(((msg.live) ? "LIVE" : "no")));

    connect(mediaUpload,SIGNAL(progress(FMessage,float)),
            ui->scrollArea,SLOT(updateProgress(FMessage,float)));
}

void ChatWindow::mediaUploadFinished(MediaUpload *mediaUpload, FMessage msg)
{
    Utilities::logData("Media Upload Finished");

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
    FMessage msg = mediaUpload->getMessage();

    disconnect(mediaUpload, 0, 0, 0);
    mediaUpload->deleteLater();

    Utilities::logData("SSL error while trying to upload a picture. Maybe bad time & date settings on the phone?");

    QMaemo5InformationBox::information(this,"There has been an SSL error while trying to upload a multimedia message.\nPlease check your phone has the correct date & time settings.",
                                       QMaemo5InformationBox::NoTimeout);

    msg.status = FMessage::Unsent;
    messageStatusUpdate(msg);
    ui->scrollArea->resetButton(msg);
}

void ChatWindow::httpErrorHandler(MediaUpload *mediaUpload, QString error)
{
    FMessage msg = mediaUpload->getMessage();

    disconnect(mediaUpload, 0, 0, 0);
    mediaUpload->deleteLater();

    Utilities::logData("Error while trying to upload a picture. " + error);

    QMaemo5InformationBox::information(this,"There has been an error while trying to upload a multimedia message.\n" + error,
                                       QMaemo5InformationBox::NoTimeout);

    msg.status = FMessage::Unsent;
    messageStatusUpdate(msg);
    ui->scrollArea->resetButton(msg);
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

    QString text;

    if (errorCode <= 18)
    {
        // This is a QAbstractSocket::SocketError error

        IOException e((QAbstractSocket::SocketError)errorCode);

        text = e.toString();
    }
    else
    {
        text = "HTTP Error " + QString::number(errorCode);
    }

    Utilities::logData("There has been an error while trying to download media. " + text);

    QMaemo5InformationBox::information(this,"There has been an error while trying to download media.\n" + text,
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
            setMute(dialog.getMuteExpireTimestamp());
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

void ChatWindow::requestPhotoUpdate(QString jid, QString photoId, bool largeFormat)
{
    emit photoUpdate(jid, photoId, largeFormat);
}

void ChatWindow::viewContact()
{
    ContactInfoWindow *window = new ContactInfoWindow(contact,this);

    connect(window,SIGNAL(photoUpdate(QString,QString,bool)),
            this,SLOT(requestPhotoUpdate(QString,QString,bool)));

    emit requestStatus(contact->jid);

    requestPhotoUpdate(contact->jid, contact->photoId, false);

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

void ChatWindow::blockOrUnblock()
{
    setBlock(!contact->blocked);
}

void ChatWindow::setBlock(bool blocked)
{
    ui->actionBlock->setText((blocked) ? "Unblock" : "Block");

    if (contact->blocked != blocked)
    {
        if (Client::connectionStatus != Client::LoggedIn)
        {
            QMaemo5InformationBox::information(this,"You have to be logged in to unblock a contact",
                                               QMaemo5InformationBox::NoTimeout);
        }
        else
        {
            contact->blocked = blocked;
            emit blockOrUnblockContact(contact->jid, blocked);
        }
    }
}

void ChatWindow::startRecording()
{
    recordingMutex.lock();

    if (ui->textEdit->document()->isEmpty())
    {
        isRecording = true;

        ui->textEdit->hide();
        ui->recordingLabel->show();
        updateRecordingTime(0);

        AudioRecorder *recorder = new AudioRecorder(Client::voiceCodec, this);

        connect(ui->sendButton,SIGNAL(released()),recorder,SLOT(stop()));
        connect(recorder,SIGNAL(finished(QString,int)),this,SLOT(finishedRecording(QString,int)));
        connect(recorder,SIGNAL(progress(int)),this,SLOT(updateRecordingTime(int)));

        myselfComposing(FMessage::Audio);

        recorder->record();

        Utilities::logData("Recording....");
    }

    recordingMutex.unlock();
}

void ChatWindow::updateRecordingTime(int current)
{
    QString duration;
    int minutes = current / 60;
    int seconds = current % 60;

    ui->recordingLabel->setText(
                "<table width=\"100%\" border=\"0\" cellspacing=\"-1\">"
                "<tr>"
                "<td width=\"50%\"><div style=\"color:red\" align=\"center\"><b>RECORDING</b></div></td>"
                "<td>" + duration.sprintf("%02d:%02d",minutes,seconds) + "<td>"
                "</tr></table>"
                );
}

void ChatWindow::finishedRecording(QString fileName, int lengthInSeconds)
{
    if (isRecording)
    {
        ui->textEdit->show();
        ui->recordingLabel->hide();

        myselfPaused();

        if (lengthInSeconds < 2)
        {
            QMaemo5InformationBox::information(this,"Press and hold the button to record, "
                                               "release to send");
            QFile file(fileName);
            file.remove();

            return;
        }

        Utilities::logData("Finished recording: " + fileName);

#ifndef Q_WS_SCRATCHBOX
        // Send media
        sendMultimediaMessage(fileName, FMessage::Audio, true, "");
#endif
    }
}

void ChatWindow::sendVoiceNotePlayed(FMessage message)
{
    if (!message.key.from_me && message.status != FMessage::Played)
    {
        message.status = FMessage::Played;

        messageStatusUpdate(message);
        emit voiceNotePlayed(message);
    }
}

void ChatWindow::forwardMessageRequested(FMessage message)
{
    emit forwardMessage(message);
}
