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

#include <QDesktopServices>

#include "accountinfowindow.h"
#include "ui_accountinfowindow.h"

#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/util/qtmd5digest.h"

#include "client.h"

AccountInfoWindow::AccountInfoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AccountInfoWindow)
{
    ui->setupUi(this);

    ui->accountStatus->setText(Client::accountstatus[0].toUpper() +
                               Client::accountstatus.mid(1));

    ui->phoneNumber->setText("+" + Client::phoneNumber);
    ui->serviceType->setText(Client::kind[0].toUpper() +
                             Client::kind.mid(1));

    ui->accountCreation->setText((Client::creation.isEmpty() ? "Unknown" :
                                    DateTimeUtilities::simpleDayFormat(
                                    Client::creation.toLongLong() * 1000)));

    ui->serviceExpiration->setText(DateTimeUtilities::simpleDayFormat(
                                     Client::expiration.toLongLong() * 1000));
}

AccountInfoWindow::~AccountInfoWindow()
{
    delete ui;
}
