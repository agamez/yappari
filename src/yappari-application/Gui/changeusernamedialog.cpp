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

#include <QMessageBox>
#include <QKeyEvent>
#include <QTimer>

#include "changeusernamedialog.h"
#include "ui_changeusernamedialog.h"
#include "globalconstants.h"

#include "Whatsapp/util/utilities.h"

#include "client.h"

ChangeUserNameDialog::ChangeUserNameDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeUserNameDialog)
{
    this->parent = parent;

    isEmojiWidgetOpen = false;

    ui->setupUi(this);

    ui->textEdit->setText(
                Utilities::WATextToHtml(
                    (Client::userName.isEmpty() ? DEFAULT_USERNAME : Client::userName),
                    32));
    ui->textEdit->setFocus();

    connect(ui->textEdit,SIGNAL(returnPressed()),this,SLOT(accept()));
    connect(ui->selectEmojiButton,SIGNAL(clicked()),this,SLOT(selectEmojiButtonClicked()));

}

ChangeUserNameDialog::~ChangeUserNameDialog()
{
    delete ui;
}

QString ChangeUserNameDialog::getUserName()
{
    return ui->textEdit->toPlainText();
}

/*

bool ChangeUserNameDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->textEdit)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = (QKeyEvent *) event;

            if (keyEvent->nativeScanCode() == 36)
            {
                QTimer::singleShot(0,this,SLOT(accept()));
                return true;
            }
        }

        else if (event->type() == QEvent::InputMethod)
        {
            QInputMethodEvent *inputEvent = (QInputMethodEvent *) event;

            //Utilities::logData("Commit String: '" + inputEvent->commitString() + "'");
            if (inputEvent->commitString() == "\n")
            {
                // Let's hide the keyboard if it was shown
                QTimer::singleShot(0,this,SLOT(closeKB()));
                QTimer::singleShot(0,this,SLOT(sendButtonClicked()));
                return true;
            }
        }

    }

    return QDialog::eventFilter(obj,event);
}
*/

void ChangeUserNameDialog::accept()
{
    QString userName = ui->textEdit->toPlainText();

    if (userName.isEmpty())
    {
        QMessageBox msg(this);

        msg.setText("Name can't be empty.");
        msg.exec();
    }
    else if (userName.length() > 25)
    {
        QMessageBox msg(this);

        msg.setText("Name can't be longer than 25 characters.");
        msg.exec();
    }
    else
        QDialog::accept();
}

void ChangeUserNameDialog::selectEmojiButtonClicked()
{
    if (!isEmojiWidgetOpen)
        openEmojiWidget();
    else
        closeEmojiWidget();
}

void ChangeUserNameDialog::openEmojiWidget()
{
    emojiWidget = new SelectEmojiWidget(parent);

    connect(emojiWidget,SIGNAL(emojiSelected(QString)),ui->textEdit,SLOT(addEmoji(QString)));

    isEmojiWidgetOpen = true;

    emojiWidget->show();
}

void ChangeUserNameDialog::closeEmojiWidget()
{
    emojiWidget->hide();
    emojiWidget->deleteLater();
    isEmojiWidgetOpen = false;
}
