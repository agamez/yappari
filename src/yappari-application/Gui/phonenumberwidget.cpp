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

#include <QFile>
#include <QRegExp>
#include <QMessageBox>
#include <QTextStream>

#include "phonenumberwidget.h"

#include "ui_phonenumberwidget.h"

#define COUNTRIES_FILE  "/usr/share/yappari/countries.tab"

PhoneNumberWidget::PhoneNumberWidget(QString ccAlpha,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PhoneNumberWidget)
{
    ui->setupUi(this);

    connect(ui->doneButton,SIGNAL(clicked()),this,SLOT(verifyPhoneNumber()));

    // Look for the numeric country code
    cc = "+1";
    QFile file(COUNTRIES_FILE);
    if (file.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList list = line.split(QChar(0x09));
            if (list[0] == ccAlpha)
                cc = "+" + list[2];
        }

        file.close();
    }

    ui->ccLineEdit->setText(cc);
    ui->numberLineEdit->setFocus();

}

PhoneNumberWidget::~PhoneNumberWidget()
{
    delete ui;
}

void PhoneNumberWidget::verifyPhoneNumber()
{
    number = ui->numberLineEdit->text();
    cc = ui->ccLineEdit->text();

    QRegExp numberRegExp("[0-9]{6,12}");
    QRegExp ccRegExp("\\+?[0-9]{1,3}");

    if (!ccRegExp.exactMatch(cc))
    {
        QMessageBox msg(this);

        msg.setText("Incorrect country code.\nPlease enter a valid country code");
        msg.exec();
    } else if (!numberRegExp.exactMatch(number))
    {
        QMessageBox msg(this);

        msg.setText("Incorrect phone number.\nPlease enter a valid phone number");
        msg.exec();
    } else {
        if (cc.left(1) == "+")
            cc = cc.right(cc.length()-1);

        QMessageBox msg(this);
        msg.setText("\nThe number +" + cc + number + " will be verified.\n\nPress Save to continue.");
        msg.setStandardButtons(QMessageBox::Save | QMessageBox::Discard );

        if (msg.exec() == QMessageBox::Save)
            emit accept(cc,number);
    }
}
