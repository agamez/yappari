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

#include <QStringList>
#include <QMessageBox>

#include "changestatusdialog.h"
#include "ui_changestatusdialog.h"

#include "client.h"

ChangeStatusDialog::ChangeStatusDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeStatusDialog)
{
    ui->setupUi(this);

    insertDefaultStatuses();
}

ChangeStatusDialog::~ChangeStatusDialog()
{
    delete ui;
}

void ChangeStatusDialog::insertDefaultStatuses()
{
    QStringList defaultStatuses;

    defaultStatuses << "Battery about to die" << "At work" << "At the movies"
                    << "At school" << "Busy" << "Available" << "I am using Yappari!";

    ui->comboBox->insertItem(0,Client::myStatus);
    ui->comboBox->insertItems(0,defaultStatuses);
}

QString ChangeStatusDialog::getStatus()
{
    return ui->comboBox->currentText();
}

void ChangeStatusDialog::accept()
{
    if (ui->comboBox->currentText().isEmpty())
    {
        QMessageBox msg(this);

        msg.setText("Status can't be empty");
        msg.exec();
    }
    else
        QDialog::accept();
}
