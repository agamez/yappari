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

#include <QSystemDeviceInfo>
#include <QTextDocument>
#include <QFile>

#include "client.h"

#include "utilities.h"
#include "qtmd5digest.h"

#include "globalconstants.h"

#define DEBUG

#define ICON_PATH   "/usr/share/yappari/icons/32x32/"

#define MIMETYPES_FILE  "/usr/share/yappari/mime-types.tab"

QHash<quint32,bool> Utilities::emojiMapping;

Utilities::Utilities()
{
}

void Utilities::logData(QString s)
{
#ifdef DEBUG
    qDebug() << qPrintable(s);

    QFile file("/home/user/.config/scorpius/yappari.log");
    if (file.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&file);
        out << qPrintable(s) << '\n';
        file.close();
    }
#endif
}

QString Utilities::decodeString(const char data[])
{
    QByteArray newData;
    for (int i = 0; data[i] != '\0'; i++) {
        newData.append(data[i] ^ 0x13);
    }
    return QString(newData);
}

QString Utilities::getChatPassword()
{
    QtMobility::QSystemDeviceInfo deviceInfo;
    QString imei = (Client::imei.isEmpty()) ? deviceInfo.imei() : Client::imei;

    logData("IMEI: " + imei);

    // ToDo: Verify imei not empty

    // Reverse the string
    QByteArray utf8  = imei.toUtf8();
    QByteArray utf8reversed;

    for (int i=0; i<utf8.length(); i++)
        utf8reversed.prepend(utf8.at(i));

    QtMD5Digest digest;
    digest.reset();

    digest.update(utf8reversed);

    QByteArray bytes = digest.digest();

    QString result;

    if (Client::android)
        result = QString::fromLatin1(bytes.toHex().constData());
    else
    {
        // Craziest conversion to hex ever
        // but this is what Whatsapp for Nokia S40 does :S

        for (int i=0; i<bytes.length(); i++)
            result.append(QString::number(((quint8)(bytes.at(i) + 128)),16));
    }

    return (result);
}

void Utilities::initEmojiMapping()
{
    QList<quint32> emojiList;

    emojiList << 0x9f8084 << 0x9f838f << 0x9f85b0 << 0x9f85b1 << 0x9f85be << 0x9f85bf << 0x9f868e << 0x9f8691 << 0x9f8692 << 0x9f8693 << 0x9f8694 << 0x9f8695 << 0x9f8696 << 0x9f8697 << 0x9f8698 << 0x9f8699 << 0x9f869a << 0x9f87a7 << 0x9f87a9 << 0x9f87aa << 0x9f87b2 << 0x9f87b3 << 0x9f87b7 << 0x9f87b8 << 0x9f87bd << 0x9f8881 << 0x9f8882 << 0x9f889a << 0x9f88af << 0x9f88b2 << 0x9f88b3 << 0x9f88b4 << 0x9f88b5 << 0x9f88b6 << 0x9f88b7 << 0x9f88b8 << 0x9f88b9 << 0x9f88ba << 0x9f8990 << 0x9f8991 << 0x9f8c80 << 0x9f8c81 << 0x9f8c82 << 0x9f8c83 << 0x9f8c84 << 0x9f8c85 << 0x9f8c86 << 0x9f8c87 << 0x9f8c88 << 0x9f8c89 << 0x9f8c8a << 0x9f8c8b << 0x9f8c8c << 0x9f8c8d << 0x9f8c8e << 0x9f8c8f << 0x9f8c90 << 0x9f8c91 << 0x9f8c92 << 0x9f8c93 << 0x9f8c94 << 0x9f8c95 << 0x9f8c96 << 0x9f8c97 << 0x9f8c98 << 0x9f8c99 << 0x9f8c9a << 0x9f8c9b << 0x9f8c9c << 0x9f8c9d << 0x9f8c9e << 0x9f8c9f << 0x9f8ca0 << 0x9f8cb0 << 0x9f8cb1 << 0x9f8cb2 << 0x9f8cb3 << 0x9f8cb4 << 0x9f8cb5 << 0x9f8cb7 << 0x9f8cb8 << 0x9f8cb9 << 0x9f8cba << 0x9f8cbb << 0x9f8cbc << 0x9f8cbd << 0x9f8cbe << 0x9f8cbf << 0x9f8d80 << 0x9f8d81 << 0x9f8d82 << 0x9f8d83 << 0x9f8d84 << 0x9f8d85 << 0x9f8d86 << 0x9f8d87 << 0x9f8d88 << 0x9f8d89 << 0x9f8d8a << 0x9f8d8b << 0x9f8d8c << 0x9f8d8d << 0x9f8d8e << 0x9f8d8f << 0x9f8d90 << 0x9f8d91 << 0x9f8d92 << 0x9f8d93 << 0x9f8d94 << 0x9f8d95 << 0x9f8d96 << 0x9f8d97 << 0x9f8d98 << 0x9f8d99 << 0x9f8d9a << 0x9f8d9b << 0x9f8d9c << 0x9f8d9d << 0x9f8d9e << 0x9f8d9f << 0x9f8da0 << 0x9f8da1 << 0x9f8da2 << 0x9f8da3 << 0x9f8da4 << 0x9f8da5 << 0x9f8da6 << 0x9f8da7 << 0x9f8da8 << 0x9f8da9 << 0x9f8daa << 0x9f8dab << 0x9f8dac << 0x9f8dad << 0x9f8dae << 0x9f8daf << 0x9f8db0 << 0x9f8db1 << 0x9f8db2 << 0x9f8db3 << 0x9f8db4 << 0x9f8db5 << 0x9f8db6 << 0x9f8db7 << 0x9f8db8 << 0x9f8db9 << 0x9f8dba << 0x9f8dbb << 0x9f8dbc << 0x9f8e80 << 0x9f8e81 << 0x9f8e82 << 0x9f8e83 << 0x9f8e84 << 0x9f8e85 << 0x9f8e86 << 0x9f8e87 << 0x9f8e88 << 0x9f8e89 << 0x9f8e8a << 0x9f8e8b << 0x9f8e8c << 0x9f8e8d << 0x9f8e8e << 0x9f8e8f << 0x9f8e90 << 0x9f8e91 << 0x9f8e92 << 0x9f8e93 << 0x9f8ea0 << 0x9f8ea1 << 0x9f8ea2 << 0x9f8ea3 << 0x9f8ea4 << 0x9f8ea5 << 0x9f8ea6 << 0x9f8ea7 << 0x9f8ea8 << 0x9f8ea9 << 0x9f8eaa << 0x9f8eab << 0x9f8eac << 0x9f8ead << 0x9f8eae << 0x9f8eaf << 0x9f8eb0 << 0x9f8eb1 << 0x9f8eb2 << 0x9f8eb3 << 0x9f8eb4 << 0x9f8eb5 << 0x9f8eb6 << 0x9f8eb7 << 0x9f8eb8 << 0x9f8eb9 << 0x9f8eba << 0x9f8ebb << 0x9f8ebc << 0x9f8ebd << 0x9f8ebe << 0x9f8ebf << 0x9f8f80 << 0x9f8f81 << 0x9f8f82 << 0x9f8f83 << 0x9f8f84 << 0x9f8f86 << 0x9f8f87 << 0x9f8f88 << 0x9f8f89 << 0x9f8f8a << 0x9f8fa0 << 0x9f8fa1 << 0x9f8fa2 << 0x9f8fa3 << 0x9f8fa4 << 0x9f8fa5 << 0x9f8fa6 << 0x9f8fa7 << 0x9f8fa8 << 0x9f8fa9 << 0x9f8faa << 0x9f8fab << 0x9f8fac << 0x9f8fad << 0x9f8fae << 0x9f8faf << 0x9f8fb0 << 0x9f9080 << 0x9f9081 << 0x9f9082 << 0x9f9083 << 0x9f9084 << 0x9f9085 << 0x9f9086 << 0x9f9087 << 0x9f9088 << 0x9f9089 << 0x9f908a << 0x9f908b << 0x9f908c << 0x9f908d << 0x9f908e << 0x9f908f << 0x9f9090 << 0x9f9091 << 0x9f9092 << 0x9f9093 << 0x9f9094 << 0x9f9095 << 0x9f9096 << 0x9f9097 << 0x9f9098 << 0x9f9099 << 0x9f909a << 0x9f909b << 0x9f909c << 0x9f909d << 0x9f909e << 0x9f909f << 0x9f90a0 << 0x9f90a1 << 0x9f90a2 << 0x9f90a3 << 0x9f90a4 << 0x9f90a5 << 0x9f90a6 << 0x9f90a7 << 0x9f90a8 << 0x9f90a9 << 0x9f90aa << 0x9f90ab << 0x9f90ac << 0x9f90ad << 0x9f90ae << 0x9f90af << 0x9f90b0 << 0x9f90b1 << 0x9f90b2 << 0x9f90b3 << 0x9f90b4 << 0x9f90b5 << 0x9f90b6 << 0x9f90b7 << 0x9f90b8 << 0x9f90b9 << 0x9f90ba << 0x9f90bb << 0x9f90bc << 0x9f90bd << 0x9f90be << 0x9f9180 << 0x9f9182 << 0x9f9183 << 0x9f9184 << 0x9f9185 << 0x9f9186 << 0x9f9187 << 0x9f9188 << 0x9f9189 << 0x9f918a << 0x9f918b << 0x9f918c << 0x9f918d << 0x9f918e << 0x9f918f << 0x9f9190 << 0x9f9191 << 0x9f9192 << 0x9f9193 << 0x9f9194 << 0x9f9195 << 0x9f9196 << 0x9f9197 << 0x9f9198 << 0x9f9199 << 0x9f919a << 0x9f919b << 0x9f919c << 0x9f919d << 0x9f919e << 0x9f919f << 0x9f91a0 << 0x9f91a1 << 0x9f91a2 << 0x9f91a3 << 0x9f91a4 << 0x9f91a5 << 0x9f91a6 << 0x9f91a7 << 0x9f91a8 << 0x9f91a9 << 0x9f91aa << 0x9f91ab << 0x9f91ac << 0x9f91ad << 0x9f91ae << 0x9f91af << 0x9f91b0 << 0x9f91b1 << 0x9f91b2 << 0x9f91b3 << 0x9f91b4 << 0x9f91b5 << 0x9f91b6 << 0x9f91b7 << 0x9f91b8 << 0x9f91b9 << 0x9f91ba << 0x9f91bb << 0x9f91bc << 0x9f91bd << 0x9f91be << 0x9f91bf << 0x9f9280 << 0x9f9281 << 0x9f9282 << 0x9f9283 << 0x9f9284 << 0x9f9285 << 0x9f9286 << 0x9f9287 << 0x9f9288 << 0x9f9289 << 0x9f928a << 0x9f928b << 0x9f928c << 0x9f928d << 0x9f928e << 0x9f928f << 0x9f9290 << 0x9f9291 << 0x9f9292 << 0x9f9293 << 0x9f9294 << 0x9f9295 << 0x9f9296 << 0x9f9297 << 0x9f9298 << 0x9f9299 << 0x9f929a << 0x9f929b << 0x9f929c << 0x9f929d << 0x9f929e << 0x9f929f << 0x9f92a0 << 0x9f92a1 << 0x9f92a2 << 0x9f92a3 << 0x9f92a4 << 0x9f92a5 << 0x9f92a6 << 0x9f92a7 << 0x9f92a8 << 0x9f92a9 << 0x9f92aa << 0x9f92ab << 0x9f92ac << 0x9f92ad << 0x9f92ae << 0x9f92af << 0x9f92b0 << 0x9f92b1 << 0x9f92b2 << 0x9f92b3 << 0x9f92b4 << 0x9f92b5 << 0x9f92b6 << 0x9f92b7 << 0x9f92b8 << 0x9f92b9 << 0x9f92ba << 0x9f92bb << 0x9f92bc << 0x9f92bd << 0x9f92be << 0x9f92bf << 0x9f9380 << 0x9f9381 << 0x9f9382 << 0x9f9383 << 0x9f9384 << 0x9f9385 << 0x9f9386 << 0x9f9387 << 0x9f9388 << 0x9f9389 << 0x9f938a << 0x9f938b << 0x9f938c << 0x9f938d << 0x9f938e << 0x9f938f << 0x9f9390 << 0x9f9391 << 0x9f9392 << 0x9f9393 << 0x9f9394 << 0x9f9395 << 0x9f9396 << 0x9f9397 << 0x9f9398 << 0x9f9399 << 0x9f939a << 0x9f939b << 0x9f939c << 0x9f939d << 0x9f939e << 0x9f939f << 0x9f93a0 << 0x9f93a1 << 0x9f93a2 << 0x9f93a3 << 0x9f93a4 << 0x9f93a5 << 0x9f93a6 << 0x9f93a7 << 0x9f93a8 << 0x9f93a9 << 0x9f93aa << 0x9f93ab << 0x9f93ac << 0x9f93ad << 0x9f93ae << 0x9f93af << 0x9f93b0 << 0x9f93b1 << 0x9f93b2 << 0x9f93b3 << 0x9f93b4 << 0x9f93b5 << 0x9f93b6 << 0x9f93b7 << 0x9f93b9 << 0x9f93ba << 0x9f93bb << 0x9f93bc << 0x9f9480 << 0x9f9481 << 0x9f9482 << 0x9f9483 << 0x9f9484 << 0x9f9485 << 0x9f9486 << 0x9f9487 << 0x9f9488 << 0x9f9489 << 0x9f948a << 0x9f948b << 0x9f948c << 0x9f948d << 0x9f948e << 0x9f948f << 0x9f9490 << 0x9f9491 << 0x9f9492 << 0x9f9493 << 0x9f9494 << 0x9f9495 << 0x9f9496 << 0x9f9497 << 0x9f9498 << 0x9f9499 << 0x9f949a << 0x9f949b << 0x9f949c << 0x9f949d << 0x9f949e << 0x9f949f << 0x9f94a0 << 0x9f94a1 << 0x9f94a2 << 0x9f94a3 << 0x9f94a4 << 0x9f94a5 << 0x9f94a6 << 0x9f94a7 << 0x9f94a8 << 0x9f94a9 << 0x9f94aa << 0x9f94ab << 0x9f94ac << 0x9f94ad << 0x9f94ae << 0x9f94af << 0x9f94b0 << 0x9f94b1 << 0x9f94b2 << 0x9f94b3 << 0x9f94b4 << 0x9f94b5 << 0x9f94b6 << 0x9f94b7 << 0x9f94b8 << 0x9f94b9 << 0x9f94ba << 0x9f94bb << 0x9f94bc << 0x9f94bd << 0x9f9590 << 0x9f9591 << 0x9f9592 << 0x9f9593 << 0x9f9594 << 0x9f9595 << 0x9f9596 << 0x9f9597 << 0x9f9598 << 0x9f9599 << 0x9f959a << 0x9f959b << 0x9f959c << 0x9f959d << 0x9f959e << 0x9f959f << 0x9f95a0 << 0x9f95a1 << 0x9f95a2 << 0x9f95a3 << 0x9f95a4 << 0x9f95a5 << 0x9f95a6 << 0x9f95a7 << 0x9f97bb << 0x9f97bc << 0x9f97bd << 0x9f97be << 0x9f97bf << 0x9f9880 << 0x9f9881 << 0x9f9882 << 0x9f9883 << 0x9f9884 << 0x9f9885 << 0x9f9886 << 0x9f9887 << 0x9f9888 << 0x9f9889 << 0x9f988a << 0x9f988b << 0x9f988c << 0x9f988d << 0x9f988e << 0x9f988f << 0x9f9890 << 0x9f9891 << 0x9f9892 << 0x9f9893 << 0x9f9894 << 0x9f9895 << 0x9f9896 << 0x9f9897 << 0x9f9898 << 0x9f9899 << 0x9f989a << 0x9f989b << 0x9f989c << 0x9f989d << 0x9f989e << 0x9f989f << 0x9f98a0 << 0x9f98a1 << 0x9f98a2 << 0x9f98a3 << 0x9f98a4 << 0x9f98a5 << 0x9f98a6 << 0x9f98a7 << 0x9f98a8 << 0x9f98a9 << 0x9f98aa << 0x9f98ab << 0x9f98ac << 0x9f98ad << 0x9f98ae << 0x9f98af << 0x9f98b0 << 0x9f98b1 << 0x9f98b2 << 0x9f98b3 << 0x9f98b4 << 0x9f98b5 << 0x9f98b6 << 0x9f98b7 << 0x9f98b8 << 0x9f98b9 << 0x9f98ba << 0x9f98bb << 0x9f98bc << 0x9f98bd << 0x9f98be << 0x9f98bf << 0x9f9980 << 0x9f9985 << 0x9f9986 << 0x9f9987 << 0x9f9988 << 0x9f9989 << 0x9f998a << 0x9f998b << 0x9f998c << 0x9f998d << 0x9f998e << 0x9f998f << 0x9f9a80 << 0x9f9a81 << 0x9f9a82 << 0x9f9a83 << 0x9f9a84 << 0x9f9a85 << 0x9f9a86 << 0x9f9a87 << 0x9f9a88 << 0x9f9a89 << 0x9f9a8a << 0x9f9a8b << 0x9f9a8c << 0x9f9a8d << 0x9f9a8e << 0x9f9a8f << 0x9f9a90 << 0x9f9a91 << 0x9f9a92 << 0x9f9a93 << 0x9f9a94 << 0x9f9a95 << 0x9f9a96 << 0x9f9a97 << 0x9f9a98 << 0x9f9a99 << 0x9f9a9a << 0x9f9a9b << 0x9f9a9c << 0x9f9a9d << 0x9f9a9e << 0x9f9a9f << 0x9f9aa0 << 0x9f9aa1 << 0x9f9aa2 << 0x9f9aa3 << 0x9f9aa4 << 0x9f9aa5 << 0x9f9aa6 << 0x9f9aa7 << 0x9f9aa8 << 0x9f9aa9 << 0x9f9aaa << 0x9f9aab << 0x9f9aac << 0x9f9aad << 0x9f9aae << 0x9f9aaf << 0x9f9ab0 << 0x9f9ab1 << 0x9f9ab2 << 0x9f9ab3 << 0x9f9ab4 << 0x9f9ab5 << 0x9f9ab6 << 0x9f9ab7 << 0x9f9ab8 << 0x9f9ab9 << 0x9f9aba << 0x9f9abb << 0x9f9abc << 0x9f9abd << 0x9f9abe << 0x9f9abf << 0x9f9b80 << 0x9f9b81 << 0x9f9b82 << 0x9f9b83 << 0x9f9b84 << 0x9f9b85 << 0xe284a2 << 0xe284b9 << 0xe28694 << 0xe28695 << 0xe28696 << 0xe28697 << 0xe28698 << 0xe28699 << 0xe286a9 << 0xe286aa << 0xe28c9a << 0xe28c9b << 0xe28fa9 << 0xe28faa << 0xe28fab << 0xe28fac << 0xe28fb0 << 0xe28fb3 << 0xe29382 << 0xe296aa << 0xe296ab << 0xe296b6 << 0xe29780 << 0xe297bb << 0xe297bc << 0xe297bd << 0xe297be << 0xe29880 << 0xe29881 << 0xe2988e << 0xe29891 << 0xe29894 << 0xe29895 << 0xe2989d << 0xe298ba << 0xe29988 << 0xe29989 << 0xe2998a << 0xe2998b << 0xe2998c << 0xe2998d << 0xe2998e << 0xe2998f << 0xe29990 << 0xe29991 << 0xe29992 << 0xe29993 << 0xe299a0 << 0xe299a3 << 0xe299a5 << 0xe299a6 << 0xe299a8 << 0xe299bb << 0xe299bf << 0xe29a93 << 0xe29aa0 << 0xe29aa1 << 0xe29aaa << 0xe29aab << 0xe29abd << 0xe29abe << 0xe29b84 << 0xe29b85 << 0xe29b8e << 0xe29b94 << 0xe29baa << 0xe29bb2 << 0xe29bb3 << 0xe29bb5 << 0xe29bba << 0xe29bbd << 0xe29c82 << 0xe29c85 << 0xe29c88 << 0xe29c89 << 0xe29c8a << 0xe29c8b << 0xe29c8c << 0xe29c8f << 0xe29c92 << 0xe29c94 << 0xe29c96 << 0xe29ca8 << 0xe29cb3 << 0xe29cb4 << 0xe29d84 << 0xe29d87 << 0xe29d8c << 0xe29d8e << 0xe29d93 << 0xe29d94 << 0xe29d95 << 0xe29d97 << 0xe29da4 << 0xe29e95 << 0xe29e96 << 0xe29e97 << 0xe29ea1 << 0xe29eb0 << 0xe29ebf << 0xe2a4b4 << 0xe2a4b5 << 0xe2ac85 << 0xe2ac86 << 0xe2ac87 << 0xe2ac9b << 0xe2ac9c << 0xe2ad90 << 0xe2ad95 << 0xe380b0 << 0xe380bd << 0xe38a97 << 0xe38a99 << 0xeca6aa << 0xeca6b7 << 0xeca6b9 << 0xeca6ba << 0xeca7a6 << 0xeca7aa << 0xeca7b7 << 0xeca8a6 << 0xeca8ad << 0xeca8ae << 0xeca8b1 << 0xeca8b2 << 0xeca8b3 << 0xeca8b4 << 0xeca8b7 << 0xeca9aa << 0xeca9bf << 0xecaaa8 << 0xecaaac << 0xecaab8 << 0xecabb7 << 0xecaca7 << 0xecacad << 0xecacb7 << 0xecadb0 << 0xecadb3 << 0xecadb7 << 0xecaea9 << 0xecaeb1 << 0xecaeb3 << 0xecaeb7 << 0xecaeb9 << 0xecafb4 << 0xecafb5 << 0xecb0b7 << 0xecb0bf << 0xecb1a7 << 0xecb2bd << 0xecb2be << 0xecb3ac << 0xecb3b1 << 0xecb5aa << 0xecb5b9 << 0xecb7ba << 0xecb8a6 << 0xecb8ac << 0xecb8be << 0xecb9ad << 0xecb9b7 << 0xecb9bc << 0xecbaa6 << 0xecbab8 << 0xecbabe << 0xecbbaa << 0xecbdaa << 0xecbfa6 << 0xee8081 << 0xee8082 << 0xee8083 << 0xee8084 << 0xee8085 << 0xee8086 << 0xee8087 << 0xee8088 << 0xee8089 << 0xee808a << 0xee808b << 0xee808c << 0xee808d << 0xee808e << 0xee808f << 0xee8090 << 0xee8091 << 0xee8092 << 0xee8093 << 0xee8094 << 0xee8095 << 0xee8096 << 0xee8097 << 0xee8098 << 0xee8099 << 0xee809a << 0xee809b << 0xee809c << 0xee809d << 0xee809e << 0xee809f << 0xee80a0 << 0xee80a1 << 0xee80a2 << 0xee80a3 << 0xee80a4 << 0xee80a5 << 0xee80a6 << 0xee80a7 << 0xee80a8 << 0xee80a9 << 0xee80aa << 0xee80ab << 0xee80ac << 0xee80ad << 0xee80ae << 0xee80af << 0xee80b0 << 0xee80b1 << 0xee80b2 << 0xee80b3 << 0xee80b4 << 0xee80b5 << 0xee80b6 << 0xee80b7 << 0xee80b8 << 0xee80b9 << 0xee80ba << 0xee80bb << 0xee80bc << 0xee80bd << 0xee80be << 0xee80bf << 0xee8180 << 0xee8181 << 0xee8182 << 0xee8183 << 0xee8184 << 0xee8185 << 0xee8186 << 0xee8187 << 0xee8188 << 0xee8189 << 0xee818a << 0xee818b << 0xee818c << 0xee818d << 0xee818e << 0xee818f << 0xee8190 << 0xee8191 << 0xee8192 << 0xee8193 << 0xee8194 << 0xee8195 << 0xee8196 << 0xee8197 << 0xee8198 << 0xee8199 << 0xee819a << 0xee8481 << 0xee8482 << 0xee8483 << 0xee8484 << 0xee8485 << 0xee8486 << 0xee8487 << 0xee8488 << 0xee8489 << 0xee848a << 0xee848b << 0xee848c << 0xee848d << 0xee848e << 0xee848f << 0xee8490 << 0xee8491 << 0xee8492 << 0xee8493 << 0xee8494 << 0xee8495 << 0xee8496 << 0xee8497 << 0xee8498 << 0xee8499 << 0xee849a << 0xee849b << 0xee849c << 0xee849d << 0xee849e << 0xee849f << 0xee84a0 << 0xee84a1 << 0xee84a2 << 0xee84a3 << 0xee84a4 << 0xee84a5 << 0xee84a6 << 0xee84a7 << 0xee84a8 << 0xee84a9 << 0xee84aa << 0xee84ab << 0xee84ac << 0xee84ad << 0xee84ae << 0xee84af << 0xee84b0 << 0xee84b1 << 0xee84b2 << 0xee84b3 << 0xee84b4 << 0xee84b5 << 0xee84b6 << 0xee84b7 << 0xee84b8 << 0xee84b9 << 0xee84ba << 0xee84bb << 0xee84bc << 0xee84bd << 0xee84be << 0xee84bf << 0xee8580 << 0xee8581 << 0xee8582 << 0xee8583 << 0xee8584 << 0xee8585 << 0xee8586 << 0xee8587 << 0xee8588 << 0xee8589 << 0xee858a << 0xee858b << 0xee858c << 0xee858d << 0xee858e << 0xee858f << 0xee8590 << 0xee8591 << 0xee8592 << 0xee8593 << 0xee8594 << 0xee8595 << 0xee8596 << 0xee8597 << 0xee8598 << 0xee8599 << 0xee859a << 0xee8881 << 0xee8882 << 0xee8883 << 0xee8884 << 0xee8885 << 0xee8886 << 0xee8887 << 0xee8888 << 0xee8889 << 0xee888a << 0xee888b << 0xee888c << 0xee888d << 0xee888e << 0xee888f << 0xee8890 << 0xee8891 << 0xee8892 << 0xee8893 << 0xee8894 << 0xee8895 << 0xee8896 << 0xee8897 << 0xee8898 << 0xee8899 << 0xee889a << 0xee889b << 0xee889c << 0xee889d << 0xee889e << 0xee889f << 0xee88a0 << 0xee88a1 << 0xee88a2 << 0xee88a3 << 0xee88a4 << 0xee88a5 << 0xee88a6 << 0xee88a7 << 0xee88a8 << 0xee88a9 << 0xee88aa << 0xee88ab << 0xee88ac << 0xee88ad << 0xee88ae << 0xee88af << 0xee88b0 << 0xee88b1 << 0xee88b2 << 0xee88b3 << 0xee88b4 << 0xee88b5 << 0xee88b6 << 0xee88b7 << 0xee88b8 << 0xee88b9 << 0xee88ba << 0xee88bb << 0xee88bc << 0xee88bd << 0xee88be << 0xee88bf << 0xee8980 << 0xee8981 << 0xee8982 << 0xee8983 << 0xee8984 << 0xee8985 << 0xee8986 << 0xee8987 << 0xee8988 << 0xee8989 << 0xee898a << 0xee898b << 0xee898c << 0xee898d << 0xee898e << 0xee898f << 0xee8990 << 0xee8991 << 0xee8992 << 0xee8993 << 0xee8c81 << 0xee8c82 << 0xee8c83 << 0xee8c84 << 0xee8c85 << 0xee8c86 << 0xee8c87 << 0xee8c88 << 0xee8c89 << 0xee8c8a << 0xee8c8b << 0xee8c8c << 0xee8c8d << 0xee8c8e << 0xee8c8f << 0xee8c90 << 0xee8c91 << 0xee8c92 << 0xee8c93 << 0xee8c94 << 0xee8c95 << 0xee8c96 << 0xee8c97 << 0xee8c98 << 0xee8c99 << 0xee8c9a << 0xee8c9b << 0xee8c9c << 0xee8c9d << 0xee8c9e << 0xee8c9f << 0xee8ca0 << 0xee8ca1 << 0xee8ca2 << 0xee8ca3 << 0xee8ca4 << 0xee8ca5 << 0xee8ca6 << 0xee8ca7 << 0xee8ca8 << 0xee8ca9 << 0xee8caa << 0xee8cab << 0xee8cac << 0xee8cad << 0xee8cae << 0xee8caf << 0xee8cb0 << 0xee8cb1 << 0xee8cb2 << 0xee8cb3 << 0xee8cb4 << 0xee8cb5 << 0xee8cb6 << 0xee8cb7 << 0xee8cb8 << 0xee8cb9 << 0xee8cba << 0xee8cbb << 0xee8cbc << 0xee8cbd << 0xee8cbe << 0xee8cbf << 0xee8d80 << 0xee8d81 << 0xee8d82 << 0xee8d83 << 0xee8d84 << 0xee8d85 << 0xee8d86 << 0xee8d87 << 0xee8d88 << 0xee8d89 << 0xee8d8a << 0xee8d8b << 0xee8d8c << 0xee8d8d << 0xee9081 << 0xee9082 << 0xee9083 << 0xee9084 << 0xee9085 << 0xee9086 << 0xee9087 << 0xee9088 << 0xee9089 << 0xee908a << 0xee908b << 0xee908c << 0xee908d << 0xee908e << 0xee908f << 0xee9090 << 0xee9091 << 0xee9092 << 0xee9093 << 0xee9094 << 0xee9095 << 0xee9096 << 0xee9097 << 0xee9098 << 0xee9099 << 0xee909a << 0xee909b << 0xee909c << 0xee909d << 0xee909e << 0xee909f << 0xee90a0 << 0xee90a1 << 0xee90a2 << 0xee90a3 << 0xee90a4 << 0xee90a5 << 0xee90a6 << 0xee90a7 << 0xee90a8 << 0xee90a9 << 0xee90aa << 0xee90ab << 0xee90ac << 0xee90ad << 0xee90ae << 0xee90af << 0xee90b0 << 0xee90b1 << 0xee90b2 << 0xee90b3 << 0xee90b4 << 0xee90b5 << 0xee90b6 << 0xee90b7 << 0xee90b8 << 0xee90b9 << 0xee90ba << 0xee90bb << 0xee90bc << 0xee90bd << 0xee90be << 0xee90bf << 0xee9180 << 0xee9181 << 0xee9182 << 0xee9183 << 0xee9184 << 0xee9185 << 0xee9186 << 0xee9187 << 0xee9188 << 0xee9189 << 0xee918a << 0xee918b << 0xee918c << 0xee918d << 0xee918e << 0xee918f << 0xee9481 << 0xee9482 << 0xee9483 << 0xee9484 << 0xee9485 << 0xee9486 << 0xee9487 << 0xee9488 << 0xee9489 << 0xee948a << 0xee948b << 0xee948c << 0xee948d << 0xee948e << 0xee948f << 0xee9490 << 0xee9491 << 0xee9492 << 0xee9493 << 0xee9494 << 0xee9495 << 0xee9496 << 0xee9497 << 0xee9498 << 0xee9499 << 0xee949a << 0xee949b << 0xee949c << 0xee949d << 0xee949e << 0xee949f << 0xee94a0 << 0xee94a1 << 0xee94a2 << 0xee94a3 << 0xee94a4 << 0xee94a5 << 0xee94a6 << 0xee94a7 << 0xee94a8 << 0xee94a9 << 0xee94aa << 0xee94ab << 0xee94ac << 0xee94ad << 0xee94ae << 0xee94af << 0xee94b0 << 0xee94b1 << 0xee94b2 << 0xee94b3 << 0xee94b4 << 0xee94b5 << 0xee94b6 << 0xee94b7;

    foreach (quint32 emojiCode, emojiList)
        emojiMapping.insert(emojiCode,true);
}

QString Utilities::WATextToHtml(QString data, int iconSize, bool urlConversion)
{
    // One time initialization
    if (emojiMapping.size() == 0)
        initEmojiMapping();

    data.replace("<","&lt;");
    data.replace(">","&gt;");
    data.replace("\n","<br>");

    QByteArray array = data.toUtf8();
    QByteArray buffer;

    //
    // PART I: EMOJI PROCESSING
    //
    QString emojiHeader = "<img ";

    if (iconSize < 32)
    {
        QString sizeStr = QString::number(iconSize);
        emojiHeader.append("width=\"" + sizeStr + "\" height=\"" + sizeStr + "\" ");
    }

    for (int i = 0; i < array.length(); i++)
    {
        uchar c = array.at(i);
        if (c == 0xEE)
        {
            if (i < array.length() - 2)
            {
                uchar e1 = array.at(i+1);
                uchar e2 = array.at(i+2);
                quint32 code = (c << 16) + (e1 << 8) + e2;
                if (emojiMapping.contains(code))
                {
                    QString emoji = emojiHeader + "src=\"" ICON_PATH +
                            QString::number(e1,16) + "-" +
                            QString::number(e2 - 0x80) + ".png\"></img>";

                    // Utilities::logData("EMOJI: " + emoji);
                    buffer.append(emoji);
                    i+=2;
                }
                else
                    buffer.append(c);
            }
            else
                buffer.append(c);

        }
        else if (c == 0xE2 || c== 0xE3)
        {
            if (i < array.length() - 2)
            {
                uchar e1 = array.at(i+1);
                uchar e2 = array.at(i+2);

                quint32 code = (c << 16) + (e1 << 8) + e2;
                if (emojiMapping.contains(code))
                {
                    QString emoji = emojiHeader + "src=\"" ICON_PATH  +
                            QString::number(c,16) + "-" +
                            QString::number(e1,16) +
                            QString::number(e2,16) + ".png\"></img>";

                    // Utilities::logData("EMOJI: " + emoji);
                    buffer.append(emoji);
                    i+=2;
                }
                else
                    buffer.append(c);
            }
            else
                buffer.append(c);
        }
        else if (c == 0xF0)
        {
            if (i < array.length() - 3)
            {
                uchar e1 = array.at(i+1);  // 0x9F
                uchar e2 = array.at(i+2);
                uchar e3 = array.at(i+3);

                // New icons: two bytes
                if (e1 == 0x9F && e2 == 0x87 && i < array.length() - 7)
                {
                    uchar f0 = array.at(i+4);
                    uchar f1 = array.at(i+5);  // 0x9F
                    uchar f2 = array.at(i+6);  // 0x87
                    uchar f3 = array.at(i+7);

                    if (f0 == 0xF0 && f1 == 0x9F && f2 == 0x87)
                    {
                        e1 = 0xEC;
                        e2 = e3;
                        e3 = f3;
                    }
                }

                quint32 code = (e1 << 16) + (e2 << 8) + e3;

                if (emojiMapping.contains(code))
                {

                    QString emoji = emojiHeader +  "src=\"" ICON_PATH +
                            QString::number(e1,16) + "-" + QString::number(e2,16) +
                            QString::number(e3,16) + ".png\"></img>";

                    // Utilities::logData("EMOJI: " + emoji);
                    buffer.append(emoji);

                    i = i + ((e1 == 0xEC) ? 7 : 3);
                }
                else
                    buffer.append(c);
            }
            else
                buffer.append(c);
        }
        else if ((c >= 0x30 && c <= 0x39) || c == 0x23)
        {
            if (i < array.length() - 3)
            {
                uchar e1 = array.at(i+1);  // 0x9F
                uchar e2 = array.at(i+2);
                uchar e3 = array.at(i+3);

                if (e1 == 0xE2 && e2 == 0x83 && e3 == 0xA3)
                {
                    uchar e;

                    if (c == 0x30)
                        e = 37;
                    else if (c == 0x23)
                        e = 16;
                    else
                        e = c - 21;

                    QString emoji = emojiHeader +  "src=\"" ICON_PATH +
                            "88-" + QString::number(e) + ".png\"></img>";

                    // Utilities::logData("EMOJI: " + emoji);
                    buffer.append(emoji);

                    i += 3;
                }
                else
                    buffer.append(c);
            }
            else
                buffer.append(c);
        }
        else if (c == 0xC2)
        {
            if (i < array.length() - 1)
            {
                uchar e1 = array.at(i+1);
                uchar e;

                if (e1 == 0xA9)
                    e = 14;
                else if (e1 == 0xAE)
                    e = 15;
                else
                {
                    buffer.append(c);
                    continue;
                }

                QString emoji = emojiHeader +  "src=\"" ICON_PATH +
                        "89-" + QString::number(e) + ".png\"></img>";

                // Utilities::logData("EMOJI: " + emoji);
                buffer.append(emoji);

                i++;
            }
            else
                buffer.append(c);
        }
        else
            buffer.append(c);
    }

    QString result = QString::fromUtf8(buffer.constData());

    //
    // PART II: URL Processing
    //

    if (urlConversion)
    {
        int pos = 0;
        QRegExp urlreg("(https?://[_a-zA-Z0-9./~:?#!$&'+,;=%-]+)");

        urlreg.setCaseSensitivity(Qt::CaseInsensitive);

        while ((pos = urlreg.indexIn(result,pos)) != -1)
        {
            QString url = "<a href=\"" + urlreg.cap(1) + "\">" +
                          shortURL(urlreg.cap(1)) + "</a>";
            result.replace(pos,urlreg.cap(1).length(),url);

            pos += url.length();
        }

        pos = 0;

        QRegExp wwwreg("(^|\\s)(www\\.[_a-zA-Z0-9./~:?#!$&'+,;=%-]+)");
        wwwreg.setCaseSensitivity(Qt::CaseInsensitive);

        while ((pos = wwwreg.indexIn(result,pos)) != -1)
        {
            QString url = "&nbsp;<a href=\"http://" + wwwreg.cap(2) + "\">" +
                          shortURL(wwwreg.cap(2)) + "</a>";
            result.replace(pos,wwwreg.cap(1).length() + wwwreg.cap(2).length(),url);

            pos += url.length();
        }
    }

    return result;
}

QString Utilities::removeEmoji(QString data)
{
    QByteArray array = data.toUtf8();
    QByteArray buffer;

    for (int i = 0; i < array.length(); i++)
    {
        uchar c = array.at(i);
        if (c == 0xEE)
        {
            if (i < array.length() - 2)
            {
                uchar e1 = array.at(i+1);
                uchar e2 = array.at(i+2);
                quint32 code = (c << 16) + (e1 << 8) + e2;
                if (emojiMapping.contains(code))
                    i+=2;
                else
                    buffer.append(c);
            }
        }
        else if (c == 0xE2 || c== 0xE3)
        {
            if (i < array.length() - 2)
            {
                uchar e1 = array.at(i+1);
                uchar e2 = array.at(i+2);

                quint32 code = (c << 16) + (e1 << 8) + e2;
                if (emojiMapping.contains(code))
                    i+=2;
                else
                    buffer.append(c);
            }
        }
        else if (c == 0xF0)
        {
            if (i < array.length() - 3)
            {
                uchar e1 = array.at(i+1);  // 0x9F
                uchar e2 = array.at(i+2);
                uchar e3 = array.at(i+3);

                // New icons: two bytes
                if (e1 == 0x9F && e2 == 0x87 && i < array.length() - 7)
                {
                    uchar f0 = array.at(i+4);
                    uchar f1 = array.at(i+5);  // 0x9F
                    uchar f2 = array.at(i+6);  // 0x87
                    uchar f3 = array.at(i+7);

                    if (f0 == 0xF0 && f1 == 0x9F && f2 == 0x87)
                    {
                        e1 = 0xEC;
                        e2 = e3;
                        e3 = f3;
                    }
                }

                quint32 code = (e1 << 16) + (e2 << 8) + e3;
                if (emojiMapping.contains(code))
                    i = i + ((e1 == 0xEC) ? 7 : 3);
                else
                    buffer.append(c);
            }
        }
        else
            buffer.append(c);
    }

    return QString::fromUtf8(buffer.constData());
}

QString Utilities::shortURL(QString url)
{
    if (url.size() > 40)
    {
        int bytesToRemove = url.size() - 40;
        int pos = url.lastIndexOf("/");

        if (pos < bytesToRemove + 6)
            pos = url.lastIndexOf(".");

        if (pos < bytesToRemove + 6)
            pos = url.size() - 6;

        url = url.left(pos - (bytesToRemove+6)) + "..." + url.right(url.size() - pos);
    }

    return url;
}

QString Utilities::guessMimeType(QString extension)
{
    QString lower = extension.toLower();

    QFile file(MIMETYPES_FILE);
    if (file.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            QStringList list = line.split(QChar(0x09));
            if (list[0] == lower && list.length() > 1)
            {
                file.close();
                return list[1];
            }
        }

        file.close();
    }

    // if this happens then Yappari was hacked
    return "application/unknown";
}

QString Utilities::getExtension(QString filename)
{
    QString lower = filename.toLower();
    int index = lower.lastIndexOf('.');

    if (index > 0 && (index + 1) < lower.length())
        return lower.mid(index+1);

    return "unknown";
}

QString Utilities::getPathFor(int media_wa_type, bool gallery)
{
    QDir home = QDir::home();
    QString folder;

    switch (media_wa_type)
    {
        case FMessage::Audio:
            folder = AUDIO_DIR;
            break;
        case FMessage::Image:
            folder = IMAGES_DIR;
            break;
        case FMessage::Video:
            folder = VIDEOS_DIR;
            break;
    }

    if (Client::importMediaToGallery || gallery)
        folder = home.path() + DEFAULT_DIR"/." + folder;
    else
        folder = home.path() + CACHE_DIR"/" + folder;

    if (!home.exists(folder))
        home.mkpath(folder);

    return folder;
}

QString Utilities::formatBytes(qint64 bytes)
{
    qreal value = bytes;

    int unit = 0;
    while (value >= 1024)
    {
        unit++;
        value /= 1024;
    }

    QString result;
    if (unit)
        result.sprintf("%4.2f ",value);
    else
        result = QString::number(value) + " ";

    switch (unit)
    {
        case 0:
            result.append("B");
            break;
        case 1:
            result.append("KB");
            break;
        case 2:
            result.append("MB");
            break;
        case 3:
            result.append("GB");
            break;
        case 4:
            result.append("TB");
            break;
        default:
            result = "#####";
            break;
    }

    return result;
}

QString Utilities::htmlToWAText(QString html)
{
    QRegExp htmlreg("(<img src=\"[^\"]+\" />)");

    int pos = 0;
    while ((pos = htmlreg.indexIn(html,pos)) != -1)
    {
        QString image = htmlreg.cap(1);

        QRegExp imagereg("([a-f0-9]+)-([a-f0-9]+).png");

        int imagePos = imagereg.indexIn(image);
        if (imagePos != -1)
        {
            QByteArray array;
            uchar e1 = imagereg.cap(1).toInt(0,16);
            if (e1 == 0x9F)
            {
                quint16 e2 = imagereg.cap(2).toInt(0,16);
                array.append(0xF0);
                array.append(e1);
                array.append((e2 & 0xFF00) >> 8);
                array.append(e2 & 0xFF);
            }
            else if (e1 == 0xEC)
            {
                quint16 e2 = imagereg.cap(2).toInt(0,16);
                array.append(0xF0);
                array.append(0x9F);
                array.append(0x87);
                array.append((e2 & 0xFF00) >> 8);
                array.append(0xF0);
                array.append(0x9F);
                array.append(0x87);
                array.append(e2 & 0xFF);
            }
            else if (e1 == 0xE2 || e1 == 0xE3)
            {
                quint16 e2 = imagereg.cap(2).toInt(0,16);
                array.append(e1);
                array.append((e2 & 0xFF00) >> 8);
                array.append(e2 & 0xFF);
            }
            else
            {
                uchar e2 = imagereg.cap(2).toInt() + 0x80;

                array.append(0xEE);
                array.append(e1);
                array.append(e2);
            }

            html.replace(pos,image.length(),QString::fromUtf8(array.constData()));
        }
    }

    QTextDocument doc;
    doc.setHtml(html);

    return doc.toPlainText();
}
