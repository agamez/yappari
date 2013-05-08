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

QString Utilities::getToken(QString phoneNumber)
{
    QString key = BUILD_KEY;
    QString buildHash = BUILD_HASH;
    QString token = key + buildHash + phoneNumber;

    QtMD5Digest digest;
    digest.reset();

    digest.update(token.toUtf8());

    QByteArray bytes = digest.digest();

    return QString::fromLatin1(bytes.toHex().constData());
}

QString Utilities::WATextToHtml(QString data, int iconSize)
{
    data.replace("<","&lt;");
    data.replace(">","&gt;");

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
                QString emoji = emojiHeader + "src=\"" ICON_PATH +
                        QString::number(e1,16) + "-" +
                        QString::number(e2 - 0x80) + ".png\"></img>";

                // Utilities::logData("EMOJI: " + emoji);
                buffer.append(emoji);
            }
            i+=2;
        }
        else if (c == 0xE2 || c== 0xE3)
        {
            if (i < array.length() - 2)
            {
                uchar e1 = array.at(i+1);
                uchar e2 = array.at(i+2);
                QString emoji = emojiHeader + "src=\"" ICON_PATH  +
                        QString::number(c,16) + "-" +
                        QString::number(e1,16) +
                        QString::number(e2,16) + ".png\"></img>";

                // Utilities::logData("EMOJI: " + emoji);
                buffer.append(emoji);
            }
            i+=2;
        }
        else if (c == 0xF0)
        {
            if (i < array.length() - 3)
            {
                uchar e1 = array.at(i+1);  // 0x9F
                uchar e2 = array.at(i+2);
                uchar e3 = array.at(i+3);
                QString emoji = emojiHeader +  "src=\"" ICON_PATH +
                        QString::number(e1,16) + "-" + QString::number(e2,16) +
                        QString::number(e3,16) + ".png\"></img>";

                // Utilities::logData("EMOJI: " + emoji);
                buffer.append(emoji);
            }
            i+=3;
        }
        else
            buffer.append(c);
    }

    QString result = QString::fromUtf8(buffer.constData());

    //
    // PART II: URL Processing
    //

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

    return result;
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
