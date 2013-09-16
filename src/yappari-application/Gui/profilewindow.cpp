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
#include <QMessageBox>
#include <QFileDialog>

#include "profilewindow.h"
#include "profilepicturewindow.h"
#include "ui_profilewindow.h"

#include "Whatsapp/util/utilities.h"

#include "globalconstants.h"

#include "client.h"

ProfileWindow::ProfileWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProfileWindow)
{
    ui->setupUi(this);
    ui->textEdit->setMainWindow(this);

    ui->centralwidget->installEventFilter(this);

    connect(ui->selectPictureButton,SIGNAL(clicked()),this,SLOT(selectPicture()));
    connect(ui->saveButton,SIGNAL(clicked()),this,SLOT(verifyUserName()));
    connect(ui->textEdit,SIGNAL(returnPressed()),this,SLOT(verifyUserName()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),
            ui->textEdit,SLOT(selectEmojiButtonClicked()));

    connect(ui->actionRemovePhoto,SIGNAL(triggered()),
            this,SLOT(removePhoto()));

    ui->nLabel->setText("Select your new name:");
    ui->textEdit->setText(Utilities::WATextToHtml(Client::userName, 32));
    ui->textEdit->moveCursor(QTextCursor::End);
    ui->textEdit->setFocus();

    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/profilephoto.png";

    if (home.exists(fileName))
    {
        QImage image(fileName);
        setPhoto(image);
    }
}

ProfileWindow::~ProfileWindow()
{
    delete ui;
}

bool ProfileWindow::eventFilter(QObject *obj, QEvent *event)
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

void ProfileWindow::verifyUserName()
{
    QString userName = Utilities::htmlToWAText(ui->textEdit->toHtml());

    if (userName.isEmpty())
    {
        QMaemo5InformationBox::information(this,"Name can't be empty",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else if (userName.length() > 25)
    {
        QMaemo5InformationBox::information(this,"Name can't be longer than 25 characters",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        // Save the alias/username
        Contact &c = Client::roster->getContact(Client::myJid);
        c.alias = userName;
        Client::roster->updateAlias(&c);

        emit changeUserName(userName);
        close();
    }
}

void ProfileWindow::selectPicture()
{
    QString mediaFolder = Utilities::getPathFor(FMessage::Image, true);

    photoFileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                 mediaFolder,
                                                 EXTENSIONS_IMAGE);

    if (!photoFileName.isNull())
    {
        QMaemo5InformationBox::information(this,"Loading image");

        QTimer::singleShot(100,this,SLOT(createProfilePictureWindow()));

    }
}

void ProfileWindow::createProfilePictureWindow()
{
    QImage photo(photoFileName);

    if (photo.width() < PREVIEW_WIDTH || photo.height() < PREVIEW_HEIGHT)
    {

        QMaemo5InformationBox::information(this,"The image is too small.  "\
                                           "Please select a photo with height and width of at least " +
                                           QString::number(PREVIEW_WIDTH) + " pixels",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        ProfilePictureWindow *window = new ProfilePictureWindow(photo,this);

        connect(window,SIGNAL(finished(QImage)),
                this,SLOT(finishedPhotoSelection(QImage)));

        window->setAttribute(Qt::WA_Maemo5StackedWindow);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowFlags(window->windowFlags() | Qt::Window);
        window->showFullScreen();
    }
}

void ProfileWindow::finishedPhotoSelection(QImage image)
{
    // Save picture in disk
    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/profilephoto.png";

    image.save(fileName);

    // Show picture in window
    setPhoto(image);

    // Save the thumbnail
    // It'll be updated in DB when the change is confirmed from WA Servers
    Contact &c = Client::roster->getContact(Client::myJid);
    c.photo = image.scaled(QSize(64,64), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    emit photoSelected(Client::myJid, image);
}


void ProfileWindow::setPhoto(QImage image)
{
    QImage preview = image.scaled(QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT),
                                  Qt::KeepAspectRatio,Qt::SmoothTransformation);

    ui->selectPictureButton->setIcon(QIcon(QPixmap::fromImage(preview)));
}

void ProfileWindow::removePhoto()
{
    QDir home = QDir::home();
    QString fileName = home.path() + CACHE_DIR"/profilephoto.png";

    if (home.exists(fileName))
        home.remove(fileName);

    ui->selectPictureButton->setIcon(QIcon("/usr/share/icons/hicolor/216x216/hildon/general_avatar2.png"));

    // Clear the thumbnail
    Contact &c = Client::roster->getContact(Client::myJid);
    c.photo = QImage();
    c.photoId.clear();

    emit photoSelected(Client::myJid, c.photo);
}
