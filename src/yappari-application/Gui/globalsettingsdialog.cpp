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

#include <QMaemo5ListPickSelector>
#include <QStandardItemModel>

#include "globalsettingsdialog.h"
#include "ui_globalsettingsdialog.h"

#include "globalconstants.h"

#include "client.h"

#define MAX_BYTES       12288

GlobalSettingsDialog::GlobalSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GlobalSettingsDialog)
{
    ui->setupUi(this);

    // Configure the Port picker

    QStandardItemModel *portModel = new QStandardItemModel(0,0,this);
    QStandardItem *item = new QStandardItem("443");
    portModel->appendRow(item);
    item = new QStandardItem("5222");
    portModel->appendRow(item);

    ui->portButton->setValueLayout(QMaemo5ValueButton::ValueUnderText);
    portSelector = new QMaemo5ListPickSelector(ui->portButton);
    portSelector->setModel(portModel);

    portSelector->setCurrentIndex((Client::port == DEFAULT_PORT) ? 0 : 1);
    ui->portButton->setPickSelector(portSelector);

    // Configure the Sync picker

    QStandardItemModel *syncModel = new QStandardItemModel(0,0,this);
    item = new QStandardItem("Enabled");
    syncModel->appendRow(item);
    item = new QStandardItem("Only numbers in international format");
    syncModel->appendRow(item);
    item = new QStandardItem("Disabled");
    syncModel->appendRow(item);

    ui->syncButton->setValueLayout(QMaemo5ValueButton::ValueUnderText);
    syncSelector = new QMaemo5ListPickSelector(ui->syncButton);
    syncSelector->setModel(syncModel);

    if (Client::sync == "intl")
        syncSelector->setCurrentIndex(1);
    else if (Client::sync == "off")
        syncSelector->setCurrentIndex(2);
    else
        syncSelector->setCurrentIndex(0);
    ui->syncButton->setPickSelector(syncSelector);

    // Configure the Sync Frequency picker

    QStandardItemModel *syncFreqModel = new QStandardItemModel(0,0,this);
    item = new QStandardItem("Every successful connection");
    syncFreqModel->appendRow(item);
    item = new QStandardItem("Once a day");
    syncFreqModel->appendRow(item);
    item = new QStandardItem("Once a week");
    syncFreqModel->appendRow(item);
    item = new QStandardItem("Once a month");
    syncFreqModel->appendRow(item);

    ui->syncFreqButton->setValueLayout(QMaemo5ValueButton::ValueUnderText);
    syncFreqSelector = new QMaemo5ListPickSelector(ui->syncFreqButton);
    syncFreqSelector->setModel(syncFreqModel);

    syncFreqSelector->setCurrentIndex(Client::syncFreq);
    ui->syncFreqButton->setPickSelector(syncFreqSelector);

    // Configure the Voice Codec picker

    QStandardItemModel *voiceCodecModel = new QStandardItemModel(0,0,this);
    item = new QStandardItem("High Quality - Medium-size files (AAC)");
    voiceCodecModel->appendRow(item);
    item = new QStandardItem("Low Quality - Tiny files (AMR)");
    voiceCodecModel->appendRow(item);

    ui->voiceCodecButton->setValueLayout(QMaemo5ValueButton::ValueUnderText);
    voiceCodecSelector = new QMaemo5ListPickSelector(ui->voiceCodecButton);
    voiceCodecSelector->setModel(voiceCodecModel);

    if (Client::voiceCodec == AMR)
        voiceCodecSelector->setCurrentIndex(1);
    else
        voiceCodecSelector->setCurrentIndex(0);
    ui->voiceCodecButton->setPickSelector(voiceCodecSelector);

    // Configure the checkboxes

    ui->nicknamesCheckBox->setChecked(Client::showNicknames);
    ui->shownumbersCheckBox->setChecked(Client::showNumbers);
    ui->popupCheckbox->setChecked(Client::popupOnFirstMessage);
    ui->importCheckbox->setChecked(Client::importMediaToGallery);
    ui->startOnBootCheckBox->setChecked(Client::startOnBoot);
    ui->enterIsSendCheckbox->setChecked(Client::enterIsSend);

    // Configure automatic download bytes

    ui->automaticDownloadLineEdit->setText(QString::number(Client::automaticDownloadBytes));
}

GlobalSettingsDialog::~GlobalSettingsDialog()
{
    delete ui;
}

quint16 GlobalSettingsDialog::getPort()
{
    return (portSelector->currentIndex() == 0) ? 443 : 5222;
}

QString GlobalSettingsDialog::getSync()
{
    switch (syncSelector->currentIndex())
    {
        case 0:
            return SYNC_ENABLED;

        case 1:
            return SYNC_INTL_ONLY;

        default:
            return SYNC_DISABLED;
    }
}

bool GlobalSettingsDialog::getShowNicknames()
{
    return ui->nicknamesCheckBox->isChecked();
}

bool GlobalSettingsDialog::getShowNumbers()
{
    return ui->shownumbersCheckBox->isChecked();
}

bool GlobalSettingsDialog::getPopupOnFirstMessage()
{
    return ui->popupCheckbox->isChecked();
}

bool GlobalSettingsDialog::getImportMediaToGallery()
{
    return ui->importCheckbox->isChecked();
}

int GlobalSettingsDialog::getSyncFrequency()
{
    return syncFreqSelector->currentIndex();
}

bool GlobalSettingsDialog::getStartOnBoot()
{
    return ui->startOnBootCheckBox->isChecked();
}

bool GlobalSettingsDialog::getEnterIsSend()
{
    return ui->enterIsSendCheckbox->isChecked();
}

int GlobalSettingsDialog::getAutomaticDownloadBytes()
{
    int bytes = ui->automaticDownloadLineEdit->text().toInt();

    if (bytes > MAX_BYTES)
        bytes = MAX_BYTES;
    else if (bytes < 0)
        bytes = 0;  // disable this feature

    return bytes;
}

QString GlobalSettingsDialog::getVoiceCodec()
{
    switch (voiceCodecSelector->currentIndex())
    {
        case 0:
            return AAC;

        case 1:
            return AMR;

        default:
            return AAC;
    }
}



