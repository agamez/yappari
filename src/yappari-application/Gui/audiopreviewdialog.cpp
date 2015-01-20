/* Copyright 2015 Alvaro Gamez Machado <alvaro.gamez@hazent.com>
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

#include "audiopreviewdialog.h"
#include "ui_audiopreviewdialog.h"

#include <QImageReader>
#include <phonon>
#include <phonon/VideoPlayer>
#include "Whatsapp/fmessage.h"

AudioPreviewDialog::AudioPreviewDialog(QWidget *parent, const QString & media_path) :
    QDialog(parent),
    ui(new Ui::AudioPreviewDialog)
{
    ui->setupUi(this);

    ui->audio->load(media_path);
    finished();
}

AudioPreviewDialog::~AudioPreviewDialog()
{
    delete ui;
}

QString AudioPreviewDialog::getCaption()
{
    return "";
}


void AudioPreviewDialog::playstop()
{
    if(ui->audio->isPlaying())
    {
        ui->audio->stop();
        finished();
    }
    else
    {
        ui->playstop->setIcon(QIcon("/usr/share/yappari/icons/48x48/stop-icon.png"));
        ui->audio->play();
    }
}

void AudioPreviewDialog::finished()
{
    ui->playstop->setIcon(QIcon("/usr/share/yappari/icons/48x48/play-icon.png"));
    ui->audio->stop();
    ui->audio->seek(0);
}
