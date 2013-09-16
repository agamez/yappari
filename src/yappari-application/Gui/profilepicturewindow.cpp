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
#include <QPainter>
#include <QTimer>

#include "profilepicturewindow.h"
#include "ui_profilepicturewindow.h"

#include "Whatsapp/util/utilities.h"

#define MAX_HEIGHT      480
#define MAX_WIDTH       800

#define MIN_PHOTO_WIDTH       224
#define MIN_PHOTO_HEIGHT      224
#define MAX_PHOTO_HEIGHT      640
#define MAX_PHOTO_WIDTH       640

#define DEFAULT_BAND_WIDTH  350
#define DEFAULT_BAND_HEIGHT 350

ProfilePictureWindow::ProfilePictureWindow(QImage photo, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProfilePictureWindow)
{
    ui->setupUi(this);

    this->photo = photo;

    // Create preview photo
    QImage preview = photo;
    if (preview.height() > MAX_HEIGHT || preview.width() > MAX_WIDTH)
    {
        preview = preview.scaled(QSize(MAX_WIDTH,MAX_HEIGHT),
                                       Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    // Create 800x480 background
    QImage background(MAX_WIDTH, MAX_HEIGHT, QImage::Format_RGB32);
    QPainter painter(&background);
    painter.setCompositionMode((QPainter::CompositionMode_SourceOver));

    screenPhotoSize = preview.size();
    screenPhotoRect = QRect(QPoint(0,0),screenPhotoSize);

    // Center preview photo in the background image
    if (preview.width() < MAX_WIDTH)
        screenPhotoRect.moveLeft( (MAX_WIDTH/2) - (preview.width()/2));

    if (preview.height() < MAX_HEIGHT)
        screenPhotoRect.moveTop( (MAX_HEIGHT/2) - (preview.height()/2));

    // Paing background image
    painter.drawImage(screenPhotoRect, preview);

    QPalette palette = this->palette();
    palette.setBrush(QPalette::Background, QBrush(background));
    setPalette(palette);

    setWindowFlags(Qt::FramelessWindowHint);

    // Create selection rectangle
    rubberBandRect.setRect(0,
                           0,
                           DEFAULT_BAND_WIDTH,
                           DEFAULT_BAND_HEIGHT);

    // Adjust rectangle if image is smaller
    if (rubberBandRect.width() > screenPhotoSize.width() ||
        rubberBandRect.height() > screenPhotoSize.height())
    {
        int newSize = screenPhotoSize.width();

        if (newSize > screenPhotoSize.height())
            newSize = screenPhotoSize.height();

        rubberBandRect.setWidth(newSize);
        rubberBandRect.setHeight(newSize);
    }

    // Center rectangle
    rubberBandRect.moveLeft( (MAX_WIDTH/2) - (rubberBandRect.width()/2));
    rubberBandRect.moveTop( (MAX_HEIGHT/2) - (rubberBandRect.height()/2));

    // Create a rubber band from the selection rectangle
    rubberBand = new QRubberBand(QRubberBand::Rectangle,this);
    rubberBand->setGeometry(rubberBandRect);
    rubberBand->show();

    connect(ui->cancelButton,SIGNAL(clicked()),this,SLOT(close()));
    connect(ui->okButton,SIGNAL(clicked()),this,SLOT(finishedSelection()));

    connect(ui->upButton,SIGNAL(clicked()),this,SLOT(increaseSelection()));
    connect(ui->downButton,SIGNAL(clicked()),this,SLOT(decreaseSelection()));

}

ProfilePictureWindow::~ProfilePictureWindow()
{
    delete ui;
}

void ProfilePictureWindow::mousePressEvent(QMouseEvent *event)
{
    if (rubberBandRect.contains(event->globalPos()))
    {
        origin = event->globalPos();

        ui->upButton->hide();
        ui->downButton->hide();
        ui->okButton->hide();
        ui->cancelButton->hide();
    }
    else
        origin = QPoint(-1,-1);
}

void ProfilePictureWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (origin != QPoint(-1,-1))
    {

        QPoint newPos = rubberBand->pos() + (event->globalPos() - origin);

        if (newPos.x() < screenPhotoRect.left())
            newPos.setX(screenPhotoRect.left());

        if (newPos.y() < screenPhotoRect.top())
            newPos.setY(screenPhotoRect.top());

        if (newPos.x() + rubberBandRect.width() > screenPhotoRect.right())
            newPos.setX(screenPhotoRect.right() - rubberBandRect.width());

        if (newPos.y() + rubberBandRect.height() > screenPhotoRect.bottom())
            newPos.setY(screenPhotoRect.bottom() - rubberBandRect.height());

        rubberBandRect.moveTo(newPos);
        rubberBand->setGeometry(rubberBandRect);
        origin = event->globalPos();
    }
}

void ProfilePictureWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    ui->upButton->show();
    ui->downButton->show();
    ui->okButton->show();
    ui->cancelButton->show();
}

QRect ProfilePictureWindow::mapSelectionToSource(QRect& selection)
{
    // Calculate aspect ratio
    qreal xratio = (qreal)photo.width() / (qreal)screenPhotoSize.width();
    qreal yratio = (qreal)photo.height() / (qreal)screenPhotoSize.height();

    return QRect((selection.x() - screenPhotoRect.left()) * xratio,
                 (selection.y() - screenPhotoRect.top()) * yratio,
                 selection.width() * xratio,
                 selection.height() * yratio);
}

void ProfilePictureWindow::finishedSelection()
{
    QMaemo5InformationBox::information(this,"Processing image");

    QTimer::singleShot(100,this,SLOT(cropImage()));
}

void ProfilePictureWindow::cropImage()
{
    // Start cropping
    QRect rect = mapSelectionToSource(rubberBandRect);

    QImage croppedImage = photo.copy(rect);

    if (croppedImage.height() > MAX_PHOTO_HEIGHT ||
        croppedImage.width() > MAX_PHOTO_WIDTH)
    {
        croppedImage = croppedImage.scaled(QSize(MAX_PHOTO_WIDTH, MAX_PHOTO_HEIGHT),
                                           Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    emit finished(croppedImage);

    close();
}

void ProfilePictureWindow::increaseSelection()
{
    QRect newRect = rubberBandRect;

    if (rubberBandRect.right() + 1 <= screenPhotoRect.right() &&
        rubberBandRect.bottom() + 1 <= screenPhotoRect.bottom() &&
        rubberBandRect.top() - 1 >= screenPhotoRect.top() &&
        rubberBandRect.right() - 1 >= screenPhotoRect.left())
    {

        newRect.setWidth(rubberBandRect.width() + 2);
        newRect.setHeight(rubberBandRect.height() + 2);

        newRect.moveTo(newRect.topLeft() - QPoint(1,1));

        rubberBandRect = newRect;
        rubberBand->setGeometry(rubberBandRect);
    }
}

void ProfilePictureWindow::decreaseSelection()
{
    QRect newRect = rubberBandRect;

    newRect.setWidth(rubberBandRect.width() - 2);
    newRect.setHeight(rubberBandRect.height() - 2);

    newRect.moveTo(newRect.topLeft() + QPoint(1,1));

    QRect sourceRect = mapSelectionToSource(newRect);
    if (sourceRect.width() >= MIN_PHOTO_WIDTH &&
        sourceRect.height() >= MIN_PHOTO_HEIGHT)
    {
        rubberBandRect = newRect;
        rubberBand->setGeometry(rubberBandRect);
    }
}

