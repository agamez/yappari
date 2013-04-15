#include "mediaselectdialog.h"
#include "ui_mediaselectdialog.h"

MediaSelectDialog::MediaSelectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MediaSelectDialog)
{
    ui->setupUi(this);

    connect(ui->imageButton,SIGNAL(clicked()),
            this,SLOT(imageSelected()));
    connect(ui->audioButton,SIGNAL(clicked()),
            this,SLOT(audioSelected()));
    connect(ui->videoButton,SIGNAL(clicked()),
            this,SLOT(videoSelected()));
}

MediaSelectDialog::~MediaSelectDialog()
{
    delete ui;
}

void MediaSelectDialog::imageSelected()
{
    mediaWaTypeSelected = FMessage::Image;
    accept();
}

void MediaSelectDialog::audioSelected()
{
    mediaWaTypeSelected = FMessage::Audio;
    accept();
}

void MediaSelectDialog::videoSelected()
{
    mediaWaTypeSelected = FMessage::Video;
    accept();
}

int MediaSelectDialog::getMediaSelected()
{
    return mediaWaTypeSelected;
}
