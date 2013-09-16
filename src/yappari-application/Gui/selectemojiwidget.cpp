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

#include <QScrollArea>
#include <QGridLayout>

#include "selectemojiwidget.h"
#include "ui_selectemojiwidget.h"

#include "Whatsapp/util/utilities.h"

#include "emojibutton.h"

#define TABS                5
#define ROWS                6

SelectEmojiWidget::SelectEmojiWidget(QWidget *parent) :
    QTabWidget(parent),
    ui(new Ui::SelectEmojiWidget)
{
    ui->setupUi(this);

    setupEmojiIcons();

    for (int i=0; i < 5; i++)
        tabSetup[i] = false;

    QString stylesheet = " QTabWidget::pane { border: 2px solid #C2C7CB; }";
    setStyleSheet(stylesheet);

    connect(this,SIGNAL(currentChanged(int)),this,SLOT(tabSelected(int)));

    setCurrentIndex(0);
}

SelectEmojiWidget::~SelectEmojiWidget()
{
    delete ui;
}

void SelectEmojiWidget::tabSelected(int index)
{
    if (!tabSetup[index])
    {
        setupEmojiTab(index);
        tabSetup[index] = true;
    }
}

void SelectEmojiWidget::setupEmojiIcons()
{
    QIcon icon;
    icon.addFile(QString::fromUtf8("/usr/share/yappari/icons/48x48/81-23.png"), QSize(), QIcon::Normal, QIcon::Off);
    setTabIcon(0,icon);

    QIcon icon1;
    icon1.addFile(QString::fromUtf8("/usr/share/yappari/icons/48x48/80-48.png"), QSize(), QIcon::Normal, QIcon::Off);
    setTabIcon(1,icon1);

    QIcon icon2;
    icon2.addFile(QString::fromUtf8("/usr/share/yappari/icons/48x48/8c-37.png"), QSize(), QIcon::Normal, QIcon::Off);
    setTabIcon(2,icon2);

    QIcon icon3;
    icon3.addFile(QString::fromUtf8("/usr/share/yappari/icons/48x48/90-46.png"), QSize(), QIcon::Normal, QIcon::Off);
    setTabIcon(3,icon3);

    QIcon icon4;
    icon4.addFile(QString::fromUtf8("/usr/share/yappari/icons/48x48/88-16.png"), QSize(), QIcon::Normal, QIcon::Off);
    setTabIcon(4,icon4);
}

void SelectEmojiWidget::setupEmojiTab(int index)
{
    // Initialize button order

    QVector<QWidget*> emojiWidgetVector;

    emojiWidgetVector.append(ui->emoji1);
    emojiWidgetVector.append(ui->emoji2);
    emojiWidgetVector.append(ui->emoji3);
    emojiWidgetVector.append(ui->emoji4);
    emojiWidgetVector.append(ui->emoji5);

    QList<QString> emojiList;

    if (index == 0)
    {
        emojiList << "90-24" << "90-23" << "9f-9897" << "9f-9899" << "84-5" << "90-9"
                  << "9f-989b" << "90-21" << "81-23" << "9f-9880" << "81-22" << "90-20"
                  << "90-5" << "84-6" << "90-13" << "90-4" << "90-3" << "90-10"
                  << "90-14" << "81-24" << "90-6" << "90-19" << "90-18" << "90-17"
                  << "90-8" << "90-1" << "90-15" << "9f-9885" << "84-8" << "9f-98a9"
                  << "9f-98ab" << "90-11" << "84-7" << "81-25" << "90-22" << "9f-98a4"
                  << "90-7" << "9f-9886" << "9f-988b" << "90-12" << "9f-988e" << "9f-98b4"
                  << "9f-98b5" << "90-16" << "9f-989f" << "9f-98a6" << "9f-98a7"
                  << "9f-9888" << "84-26" << "9f-98ae" << "9f-98ac" << "9f-9890"
                  << "9f-9895" << "9f-98af" << "9f-98b6" << "9f-9887" << "90-2"
                  << "9f-9891" << "94-22" << "94-23" << "85-18" << "94-27" << "94-30"
                  << "94-26" << "80-1" << "80-2" << "80-4" << "80-5" << "94-24"
                  << "94-25" << "94-21" << "81-14" << "94-28" << "9f-98ba" << "9f-98b8"
                  << "9f-98bb" << "9f-98bd" << "9f-98bc" << "9f-9980" << "9f-98bf"
                  << "9f-98b9" << "9f-98be" << "9f-91b9" << "9f-91ba" << "9f-9988"
                  << "9f-9989" << "9f-998a" << "84-28" << "84-12" << "81-26" << "84-29"
                  << "8c-46" << "8c-53" << "9f-92ab" << "9f-92a5" << "8c-52" << "8c-49"
                  << "9f-92a7" << "84-60" << "8c-48" << "90-27" << "90-25" << "90-26"
                  << "9f-9185" << "90-28" << "80-14" << "90-33" << "90-32" << "80-13"
                  << "80-16" << "80-17" << "90-30" << "80-18" << "90-34" << "88-46"
                  << "88-47" << "88-49" << "88-48" << "90-39" << "90-29" << "80-15"
                  << "90-31" << "85-12" << "88-1" << "84-21" << "94-31" << "90-40"
                  << "9f-91aa" << "9f-91ac" << "9f-91ad" << "84-17" << "90-37"
                  << "90-41" << "90-36" << "90-35" << "89-19" << "9f-998b"
                  << "8c-30" << "8c-31" << "8c-29" << "9f-91b0" << "9f-998e" << "9f-998d"
                  << "90-38" << "94-3" << "84-14" << "8c-24" << "80-7" << "9f-919e"
                  << "8c-26" << "84-62" << "8c-27" << "80-6" << "8c-2" << "9f-919a"
                  << "8c-25" << "9f-8ebd" << "9f-9196" << "8c-33" << "8c-34" << "84-30"
                  << "8c-35" << "9f-919d" << "9f-919b" << "9f-9193" << "8c-20" << "90-60"
                  << "8c-28" << "8c-44" << "8c-42" << "8c-45" << "8c-43" << "80-34"
                  << "80-35" << "8c-40" << "8c-39" << "9f-9295" << "9f-9296" << "9f-929e"
                  << "8c-41" << "9f-928c" << "80-3" << "80-52" << "80-53"
                  << "9f-91a4" << "9f-91a5" << "9f-92ac" << "94-54" << "9f-92ad";
    }
    else if (index == 1)
    {
        emojiList << "81-18" << "94-42" << "81-15" << "81-19" << "94-36" << "94-44"
                  << "94-49" << "81-16" << "94-39" << "81-17" << "84-11" << "9f-90bd"
                  << "94-43" << "94-47" << "84-9" << "94-40" << "80-26" << "94-41"
                  << "94-38" << "9f-90bc" << "81-21" << "94-33" << "94-35" << "9f-90a5"
                  << "9f-90a3" << "94-46" << "94-45" << "9f-90a2" << "94-37" << "9f-909d"
                  << "9f-909c" << "9f-909e" << "9f-908c" << "84-10" << "91-1" << "94-34"
                  << "80-25" << "94-32" << "81-20" << "9f-908b" << "9f-9084" << "9f-908f"
                  << "9f-9080" << "9f-9083" << "9f-9085" << "9f-9087" << "9f-9089"
                  << "9f-9090" << "9f-9093" << "9f-9095" << "9f-9096" << "9f-9081"
                  << "9f-9082" << "9f-90b2" << "9f-90a1" << "9f-908a" << "94-48"
                  << "9f-90aa" << "9f-9086" << "9f-9088" << "9f-90a9" << "9f-90be"
                  << "8c-6" << "80-48" << "8c-4" << "84-16" << "80-50" << "8c-5"
                  << "8c-3" << "84-24" << "91-7" << "84-25" << "9f-8cbf" << "91-4"
                  << "9f-8d84" << "8c-8" << "8c-7" << "9f-8cb2" << "9f-8cb3" << "9f-8cb0"
                  << "9f-8cb1" << "9f-8cbc" << "9f-8c90" << "9f-8c9e" << "9f-8c9d"
                  << "9f-8c9a" << "9f-8c91" << "9f-8c92" << "9f-8c93" << "9f-8c94"
                  << "9f-8c95" << "9f-8c96" << "9f-8c97" << "9f-8c98" << "9f-8c9c"
                  << "9f-8c9b" << "81-12" << "9f-8c8d" << "9f-8c8e" << "9f-8c8f"
                  << "9f-8c8b" << "9f-8c8c" << "9f-8ca0" << "8c-47" << "81-10"
                  << "e2-9b85" << "81-9" << "84-61" << "81-11" << "e2-9d84" << "81-8"
                  << "91-3" << "9f-8c81" << "91-12" << "90-62";

    }
    else if (index == 2)
    {
        emojiList << "90-54" << "90-55" << "90-56" << "90-58" << "90-57" << "90-59"
                  << "84-23" << "91-0" << "91-2" << "91-6" << "91-5" << "84-27" << "91-8"
                  << "80-51" << "84-18" << "9f-8e8b" << "8c-18" << "9f-8e8a" << "8c-16"
                  << "85-3" << "9f-94ae" << "80-61" << "80-8" << "9f-93b9" << "84-41"
                  << "84-38" << "84-39" << "8c-22" << "9f-92be" << "80-12" << "80-10"
                  << "80-9" << "9f-939e" << "9f-939f" << "80-11" << "85-11" << "84-42"
                  << "84-40" << "85-1" << "9f-9489" << "9f-9488" << "9f-9487" << "8c-37"
                  << "9f-9495" << "85-2" << "8c-23" << "e2-8fb3" << "e2-8c9b"
                  << "e2-8fb0" << "e2-8c9a" << "85-5" << "85-4" << "9f-948f" << "9f-9490"
                  << "80-63" << "9f-948e" << "84-15" << "9f-94a6" << "9f-9486"
                  << "9f-9485" << "9f-948c" << "9f-948b" << "84-20" << "9f-9b81"
                  << "84-63" << "9f-9abf" << "85-0" << "9f-94a7" << "9f-94a9" << "84-22"
                  << "9f-9aaa" << "8c-14" << "8c-17" << "84-19" << "9f-94aa" << "8c-15"
                  << "84-59" << "84-47" << "9f-92b4" << "9f-92b5" << "9f-92b7"
                  << "9f-92b6" << "9f-92b3" << "9f-92b8" << "84-4" << "9f-93a7"
                  << "9f-93a5" << "9f-93a4" << "e2-9c89" << "84-3" << "9f-93a8"
                  << "9f-93af" << "84-1" << "9f-93aa" << "9f-93ac" << "9f-93ad"
                  << "84-2" << "9f-93a6" << "8c-1" << "9f-9384" << "9f-9383" << "9f-9391"
                  << "9f-938a" << "9f-9388" << "9f-9389" << "9f-939c" << "9f-938b"
                  << "9f-9385" << "9f-9386" << "9f-9387" << "9f-9381" << "9f-9382"
                  << "8c-19" << "9f-938c" << "9f-938e" << "e2-9c92" << "e2-9c8f"
                  << "9f-938f" << "9f-9390" << "9f-9395" << "9f-9397" << "9f-9398"
                  << "9f-9399" << "9f-9393" << "9f-9394" << "9f-9392" << "9f-939a"
                  << "85-8" << "9f-9496" << "9f-939b" << "9f-94ac" << "9f-94ad"
                  << "9f-93b0" << "94-2" << "8c-36" << "80-60" << "8c-10" << "9f-8ebc"
                  << "80-62" << "8c-38" << "9f-8eb9" << "9f-8ebb" << "81-2" << "81-0"
                  << "81-1" << "84-43" << "9f-8eae" << "9f-838f" << "9f-8eb4" << "84-45"
                  << "9f-8eb2" << "84-48" << "90-43" << "90-42" << "80-24" << "80-22"
                  << "80-21" << "90-44" << "9f-8f89" << "9f-8eb3" << "80-20" << "9f-9ab5"
                  << "9f-9ab4" << "84-50" << "9f-8f87" << "84-49" << "80-19" << "9f-8f82"
                  << "90-45" << "80-23" << "9f-8ea3" << "81-5" << "8c-56" << "8c-11"
                  << "9f-8dbc" << "81-7" << "8c-12" << "81-4" << "9f-8db9" << "9f-8db7"
                  << "81-3" << "9f-8d95" << "84-32" << "8c-59" << "9f-8d97" << "9f-8d96"
                  << "8c-63" << "8d-1" << "9f-8da4" << "8d-12" << "8d-4" << "9f-8da5"
                  << "8d-2" << "8c-61" << "8c-62" << "8d-0" << "8d-13" << "8d-3"
                  << "8c-60" << "85-7" << "8c-57" << "9f-8da9" << "9f-8dae" << "8c-58"
                  << "9f-8da8" << "90-63" << "8d-11" << "81-6" << "9f-8daa" << "9f-8dab"
                  << "9f-8dac" << "9f-8dad" << "9f-8daf" << "8d-5" << "9f-8d8f" << "8d-6"
                  << "9f-8d8b" << "9f-8d92" << "9f-8d87" << "8d-8" << "8d-7" << "9f-8d91"
                  << "9f-8d88" << "9f-8d8c" << "9f-8d90" << "9f-8d8d" << "9f-8da0"
                  << "8d-10" << "8d-9" << "9f-8cbd";

    }
    else if (index == 3)
    {
        emojiList << "80-54" << "9f-8fa1" << "85-23" << "80-56" << "85-19" << "85-21"
                  << "85-13" << "85-22" << "94-1" << "85-24" << "90-61" << "80-55"
                  << "94-4" << "9f-8fa4" << "91-10" << "85-6" << "94-5" << "94-6"
                  << "84-34" << "94-8" << "94-9" << "9f-97be" << "80-59" << "81-13"
                  << "91-9" << "91-11" << "94-29" << "9f-8c89" << "9f-8ea0" << "84-36"
                  << "84-33" << "90-51" << "88-2" << "80-28" << "84-53" << "9f-9aa3"
                  << "e2-9a93" << "84-13" << "80-29" << "84-31" << "9f-9a81" << "9f-9a82"
                  << "9f-9a8a" << "80-57" << "9f-9a9e" << "9f-9a86" << "90-53" << "80-31"
                  << "9f-9a88" << "90-52" << "9f-9a9d" << "9f-9a8b" << "80-30"
                  << "9f-9a8e" << "85-25" << "9f-9a8d" << "90-46" << "9f-9a98" << "80-27"
                  << "85-26" << "9f-9a96" << "9f-9a9b" << "90-47" << "9f-9aa8" << "90-50"
                  << "9f-9a94" << "90-48" << "90-49" << "9f-9a90" << "84-54" << "9f-9aa1"
                  << "9f-9a9f" << "9f-9aa0" << "9f-9a9c" << "8c-32" << "85-16" << "84-37"
                  << "9f-9aa6" << "85-14" << "89-18" << "84-55" << "88-9" << "80-58"
                  << "9f-8fae" << "84-51" << "84-35" << "9f-97bf" << "9f-8eaa"
                  << "9f-8ead" << "9f-938d" << "9f-9aa9" << "94-11" << "94-20" << "94-14"
                  << "94-19" << "94-12" << "94-13" << "94-17" << "94-15" << "94-18"
                  << "94-16";

    }
    else if (index == 4)
    {
        emojiList << "88-28" << "88-29" << "88-30" << "88-31" << "88-32" << "88-33"
                  << "88-34" << "88-35" << "88-36" << "88-37" << "9f-949f" << "9f-94a2"
                  << "88-16" << "9f-94a3" << "88-50" << "88-51" << "88-53" << "88-52"
                  << "9f-94a0" << "9f-94a1" << "9f-94a4" << "88-54" << "88-55" << "88-56"
                  << "88-57" << "e2-8694" << "e2-8695" << "9f-9484" << "88-59" << "88-58"
                  << "9f-94bc" << "9f-94bd" << "e2-86a9" << "e2-86aa" << "e2-84b9"
                  << "88-61" << "88-60" << "e2-8fab" << "e2-8fac" << "e2-a4b5"
                  << "e2-a4b4" << "89-13" << "9f-9480" << "9f-9481" << "9f-9482"
                  << "88-18" << "88-19" << "88-20" << "9f-8693" << "9f-8696" << "88-11"
                  << "94-7" << "88-3" << "88-44" << "88-43" << "88-42" << "9f-88b4"
                  << "9f-88b2" << "88-38" << "88-39" << "88-45" << "88-21" << "88-22"
                  << "85-17" << "84-56" << "84-57" << "84-58" << "8c-9" << "9f-9ab0"
                  << "9f-9aae" << "85-15" << "88-10" << "88-8" << "88-23" << "88-24"
                  << "88-40" << "e2-9382" << "9f-9b82" << "9f-9b84" << "9f-9b85"
                  << "9f-9b83" << "9f-8991" << "8c-21" << "8c-13" << "9f-8691"
                  << "9f-8698" << "88-41" << "9f-9aab" << "88-7" << "9f-93b5"
                  << "9f-9aaf" << "9f-9ab1" << "9f-9ab3" << "9f-9ab7" << "9f-9ab8"
                  << "e2-9b94" << "88-6" << "e2-9d87" << "e2-9d8e" << "e2-9c85"
                  << "88-5" << "88-4" << "84-46" << "89-16" << "89-17" << "94-50"
                  << "94-51" << "94-52" << "94-53" << "9f-92a0" << "88-17" << "e2-99bb"
                  << "88-63" << "89-0" << "89-1" << "89-2" << "89-3" << "89-4" << "89-5"
                  << "89-6" << "89-7" << "89-8" << "89-9" << "89-10" << "89-11"
                  << "88-62" << "85-20" << "85-10" << "9f-92b2" << "85-9" << "89-14"
                  << "89-15" << "94-55" << "84-44" << "e3-80b0" << "89-12" << "9f-949a"
                  << "9f-9499" << "9f-949b" << "9f-949c" << "8c-51" << "8c-50" << "80-33"
                  << "80-32" << "8c-55" << "8c-54" << "9f-9483" << "80-47" << "9f-95a7"
                  << "80-36" << "9f-959c" << "80-37" << "9f-959d" << "80-38" << "9f-959e"
                  << "80-39" << "9f-959f" << "80-40" << "9f-95a0" << "80-41" << "80-42"
                  << "80-43" << "80-44" << "80-45" << "80-46" << "9f-95a1" << "9f-95a2"
                  << "9f-95a3" << "9f-95a4" << "9f-95a5" << "9f-95a6" << "e2-9c96"
                  << "e2-9e95" << "e2-9e96" << "e2-9e97" << "88-14" << "88-12" << "88-15"
                  << "88-13" << "9f-92ae" << "9f-92af" << "e2-9c94" << "e2-9891"
                  << "9f-9498" << "9f-9497" << "e2-9eb0" << "80-49" << "88-26" << "88-27"
                  << "e2-97bc" << "e2-97bb" << "e2-97be" << "e2-97bd" << "e2-96aa"
                  << "e2-96ab" << "9f-94ba" << "e2-ac9c" << "e2-ac9b" << "e2-9aab"
                  << "88-25" << "9f-94b5" << "9f-94bb" << "9f-94b6" << "9f-94b7"
                  << "9f-94b8" << "9f-94b9";
    }

    QScrollArea *scrollArea = new QScrollArea(emojiWidgetVector.at(index));
    scrollArea->setGeometry(QRect(0, 0, 520, 255));
    QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(scrollArea->sizePolicy().hasHeightForWidth());
    scrollArea->setSizePolicy(sizePolicy);
    scrollArea->setFrameShadow(QFrame::Raised);
    scrollArea->setLineWidth(2);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(false);

    QWidget *scrollAreaWidgetContents = new QWidget(scrollArea);
    QWidget *layoutWidget = new QWidget(scrollAreaWidgetContents);

    QGridLayout *gridLayout = new QGridLayout(layoutWidget);
    gridLayout->setSpacing(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    gridLayout->setContentsMargins(0, 0, 0, 0);

    int i = 0;
    for (int row = 0; i < emojiList.length(); row ++)
        for (int col = 0; i < emojiList.length() && col < ROWS; col ++)
        {
            EmojiButton *emoji = new EmojiButton(emojiList.at(i),layoutWidget);
            emoji->setObjectName("e" + emojiList.at(i++));
            sizePolicy.setHeightForWidth(emoji->sizePolicy().hasHeightForWidth());
            emoji->setSizePolicy(sizePolicy);
            emoji->setMinimumSize(QSize(85, 80));

            gridLayout->addWidget(emoji, row, col, 1, 1);

            connect(emoji,SIGNAL(emojiSelected(QString)),this,SLOT(sendEmojiSelected(QString)));
        }

    scrollArea->setWidget(layoutWidget);

    scrollArea->show();
}

void SelectEmojiWidget::sendEmojiSelected(QString path)
{
    emit emojiSelected(path);
}
