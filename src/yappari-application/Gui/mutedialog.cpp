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

#include <QDateTime>

#include "mutedialog.h"
#include "ui_mutedialog.h"

MuteDialog::MuteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MuteDialog)
{
    ui->setupUi(this);

    QStringList periods;

    periods << "Permanent" << "1 Hour" << "8 Hours" << "1 Day"
            << "1 Week";

    ui->comboBox->insertItems(0,periods);
    ui->comboBox->setCurrentIndex(0);
}

MuteDialog::~MuteDialog()
{
    delete ui;
}

qint64 MuteDialog::getMuteExpireTimestamp()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    switch(ui->comboBox->currentIndex())
    {
        case 0:
            return 0;
            break;

        case 1:
            return now + 3600000;

        case 2:
            return now + (8 * 3600000);
            break;

        case 3:
            return now + (24 * 3600000);
            break;

        case 4:
            return now + (7 * 24 * 3600000);
            break;
    }

    // It will never reach here
    return 0;
}
