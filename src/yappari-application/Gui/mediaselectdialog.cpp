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
