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

#include <QDesktopServices>
#include <QUrl>

#include "whatsnewwindow.h"
#include "ui_whatsnewwindow.h"

#include "globalconstants.h"

#include "version.h"

#define TEXT "<b>Thank you for installing Yappari!</b><br><br>"\
    "If you like this application and want to see more updates consider contributing to this project by clicking on the Donate button.<br><br>"\
    "<b>New in this version:</b><br>"\
    "<li>&bull; Implemented new WhatsApp multimedia upload method.</li>"\
    "<li>&bull; Fixed all crashes I know of.</li>"\
    "<li>&bull; Fixed offline multimedia reception bug.</li>"

WhatsNewWindow::WhatsNewWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WhatsNewWindow)
{
    ui->setupUi(this);

    ui->yappariText->setText("Yappari "FULL_VERSION);

    ui->textBrowser->setHtml(TEXT);

    connect(ui->donateButton,SIGNAL(released()),this,SLOT(donateAction()));
}

WhatsNewWindow::~WhatsNewWindow()
{
    delete ui;
}

void WhatsNewWindow::donateAction()
{
    QDesktopServices::openUrl(QUrl(DONATE_URL));
}
