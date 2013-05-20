#include <QMaemo5InformationBox>
#include <QFileDialog>

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/fmessage.h"

#include "creategroupwindow.h"
#include "selectgroupparticipantswindow.h"
#include "profilepicturewindow.h"
#include "ui_creategroupwindow.h"

#include "globalconstants.h"

CreateGroupWindow::CreateGroupWindow(ContactRoster *roster,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CreateGroupWindow)
{
    this->roster = roster;

    ui->setupUi(this);
    ui->textEdit->setMainWindow(this);

    ui->centralwidget->installEventFilter(this);

    connect(ui->selectPictureButton,SIGNAL(clicked()),this,SLOT(selectPicture()));
    connect(ui->nextButton,SIGNAL(clicked()),this,SLOT(verifyGroupName()));
    connect(ui->textEdit,SIGNAL(returnPressed()),this,SLOT(verifyGroupName()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),
            ui->textEdit,SLOT(selectEmojiButtonClicked()));

    connect(ui->actionRemoveGroupIcon,SIGNAL(triggered()),
            this,SLOT(removePhoto()));

    ui->textEdit->setFocus();
}


CreateGroupWindow::~CreateGroupWindow()
{
    delete ui;
}

bool CreateGroupWindow::eventFilter(QObject *obj, QEvent *event)
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

void CreateGroupWindow::verifyGroupName()
{
    QString groupName = Utilities::htmlToWAText(ui->textEdit->toHtml());

    if (groupName.isEmpty())
    {
        QMaemo5InformationBox::information(this,"Name can't be empty",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else if (groupName.length() > 25)
    {
        QMaemo5InformationBox::information(this,"Name can't be longer than 25 characters",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        SelectGroupParticipantsWindow *window =
                new SelectGroupParticipantsWindow(roster,this);

        connect(window,SIGNAL(createGroup(QStringList)),
                this,SLOT(createGroup(QStringList)));

        window->setAttribute(Qt::WA_Maemo5StackedWindow);
        window->setAttribute(Qt::WA_DeleteOnClose);
        window->setWindowFlags(window->windowFlags() | Qt::Window);
        window->show();
    }
}

void CreateGroupWindow::selectPicture()
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

void CreateGroupWindow::createProfilePictureWindow()
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

void CreateGroupWindow::finishedPhotoSelection(QImage image)
{
    this->photo = image;

    setPhoto(image);
}


void CreateGroupWindow::setPhoto(QImage image)
{
    QImage preview = image.scaled(QSize(PREVIEW_WIDTH,PREVIEW_HEIGHT),
                                  Qt::KeepAspectRatio,Qt::SmoothTransformation);

    ui->selectPictureButton->setIcon(QIcon(QPixmap::fromImage(preview)));
}

void CreateGroupWindow::removePhoto()
{
    this->photo = QImage();
    ui->selectPictureButton->setIcon(QIcon("/usr/share/icons/hicolor/216x216/hildon/general_conference_avatar.png"));
}

void CreateGroupWindow::createGroup(QStringList participants)
{
    QString groupName = Utilities::htmlToWAText(ui->textEdit->toHtml());

    emit createGroupChat(photo, groupName, participants);
    close();
}
