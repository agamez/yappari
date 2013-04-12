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

#include "globalconstants.h"
#include "client.h"

#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/util/utilities.h"

#include "chatimageitem.h"
#include "ui_chatimageitem.h"

ChatImageItem::ChatImageItem(FMessage message, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChatImageItem)
{
    ui->setupUi(this);

    this->message = message;

    QImage image = QImage::fromData(message.data);

    ui->image->setPixmap(QPixmap::fromImage(image));

    connect(ui->viewImageButton,SIGNAL(clicked()),this,SLOT(viewImageInBrowser()));

    setNickname(message);
    setTimestamp(message);
}


void ChatImageItem::setNickname(FMessage message)
{
    bool from_me = message.key.from_me;

    QString from = (from_me)
            ? "You"
            : message.notify_name.replace("<","&lt;").replace(">","&gt;");

    QString nickcolor = Client::nickcolor.isEmpty() ? "cyan" : Client::nickcolor;
    QString mycolor = Client::mycolor.isEmpty() ? "#333333" : Client::mycolor;

    ui->groupBox->setStyleSheet("QGroupBox { border-color: "
                                + mycolor +
                                "; border-width: 1px; "
                                "border-style: solid; }");

    QString html = "<div style=\"color:"
                   + nickcolor + "\">" +
                   from + ":</div>";

    ui->nickname->setTextFormat(Qt::RichText);
    ui->nickname->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    ui->nickname->setText(html);
}


void ChatImageItem::setTimestamp(FMessage message)
{
    bool from_me = message.key.from_me;

    QString day;

    if (!DateTimeUtilities::isSameDate(message.timestamp,
                                       QDateTime::currentMSecsSinceEpoch()))
        day = DateTimeUtilities::shortDayFormat(message.timestamp);


    QString time = DateTimeUtilities::shortTimeFormat(message.timestamp);

    QString html = "<div style=\"font-size:18px;color:gray\">"
                    + day + " </div>";

    if (from_me)
    {
        html.append("<div align=\"right\">");

        if (message.status == FMessage::ReceivedByServer)
            html.append(CHECK);
        else if (message.status == FMessage::ReceivedByTarget)
           html.append(DOUBLECHECK);
        else if (message.status == FMessage::Uploading)
           html.append(UPLOADING);
        else
           html.append(GRAYCHECK);

        html.append("</div>");
    }

    html.append("<div style=\"font-size:18px;color:gray\">" + time +
                "</div>");

    ui->timestamp->setTextFormat(Qt::RichText);
    ui->timestamp->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    ui->timestamp->setText(html);

}

ChatImageItem::~ChatImageItem()
{
    delete ui;
}

void ChatImageItem::viewImageInBrowser()
{
    QDesktopServices::openUrl(QUrl(message.media_url));
}

void ChatImageItem::updateTimestamp()
{
    Utilities::logData("ChatImageItem: updateTimestamp()");
    setTimestamp(message);
}
