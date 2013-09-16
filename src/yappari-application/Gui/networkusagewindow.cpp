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

#include "networkusagewindow.h"
#include "ui_networkusagewindow.h"

#include "Whatsapp/util/datacounters.h"
#include "Whatsapp/util/utilities.h"

#include "client.h"

NetworkUsageWindow::NetworkUsageWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::NetworkUsageWindow)
{
    ui->setupUi(this);

    model = new QStandardItemModel(ui->tableView);
    ui->tableView->setModel(model);

    QHeaderView *headersView = new QHeaderView(Qt::Horizontal,ui->tableView);

    headersView->setResizeMode(QHeaderView::Stretch);

    ui->tableView->setHorizontalHeader(headersView);
    ui->tableView->verticalHeader()->setVisible(false);

    QStringList labels;
    labels << "Counter" << "Received" << "Sent";

    model->setHorizontalHeaderLabels(labels);

    refresh();

    connect(ui->actionRefresh,SIGNAL(triggered()),this,SLOT(refresh()));
    connect(ui->actionReset,SIGNAL(triggered()),this,SLOT(reset()));

}

NetworkUsageWindow::~NetworkUsageWindow()
{
    delete ui;
}

void NetworkUsageWindow::reset()
{
    // Confirmation dialog
    QMessageBox msg(this);
    msg.setText("Are you sure you want to reset the counters?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    if (msg.exec() == QMessageBox::Yes)
    {
        Client::dataCounters.resetCounters();
        refresh();
    }
}

void NetworkUsageWindow::refresh()
{
    for (int i = 0; i < TOTAL_COUNTERS; i++)
    {
        DataCounters::CounterType counter = (DataCounters::CounterType) i;
        QString counterLabel;
        switch (counter)
        {
            case DataCounters::Messages:
                counterLabel = "Messages";
                break;
            case DataCounters::MessageBytes:
                counterLabel = "Messages bytes";
                break;
            case DataCounters::ImageBytes:
                counterLabel = "Images bytes";
                break;
            case DataCounters::VideoBytes:
                counterLabel = "Videos bytes";
                break;
            case DataCounters::AudioBytes:
                counterLabel = "Audio bytes";
                break;
            case DataCounters::ProfileBytes:
                counterLabel = "Profile pictures bytes";
                break;
            case DataCounters::SyncBytes:
                counterLabel = "Synchronization bytes";
                break;
            case DataCounters::ProtocolBytes:
                counterLabel = "Protocol bytes";
                break;
            case DataCounters::Total:
                counterLabel = "Total bytes";
                break;
        }

        int column = 0;
        QStandardItem *item = new QStandardItem(counterLabel);
        model->setItem(i, column++, item);
        item->setSelectable(false);
        qint64 bytes = Client::dataCounters.getReceivedBytes(i);
        QString str = (i > 0) ? Utilities::formatBytes(bytes)
                                : QString::number(bytes);
        item = new QStandardItem(str);
        item->setSelectable(false);
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(i, column++, item);
        bytes = Client::dataCounters.getSentBytes(i);
        str = (i > 0) ? Utilities::formatBytes(bytes)
                      : QString::number(bytes);
        item = new QStandardItem(str);
        item->setSelectable(false);
        item->setTextAlignment(Qt::AlignCenter);
        model->setItem(i, column++, item);

    }
}
