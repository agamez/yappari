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

#include <QLocale>
#include <QRegExp>

#include "datetimeutilities.h"

#include "utilities.h"


DateTimeUtilities::DateTimeUtilities()
{
}

bool DateTimeUtilities::isSameDate(qint64 d1, qint64 d2)
{
    QLocale locale = QLocale::system();
    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(d1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(d2);

    return (dt1.toString(locale.dateFormat()) == dt2.toString(locale.dateFormat()));
}

bool DateTimeUtilities::isYesterdayOf(qint64 d1, qint64 d2)
{
    QLocale locale = QLocale::system();
    d2 = d2 - 86400000;
    QDateTime dt1 = QDateTime::fromMSecsSinceEpoch(d1);
    QDateTime dt2 = QDateTime::fromMSecsSinceEpoch(d2);

    return (dt1.toString(locale.dateFormat()) == dt2.toString(locale.dateFormat()));
}

QString DateTimeUtilities::simpleDayFormat(qint64 d)
{
    QLocale locale = QLocale::system();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(d);
    return (dt.toString(locale.dateFormat()));
}

QString DateTimeUtilities::shortDayFormat(qint64 d)
{
    QLocale locale = QLocale::system();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(d);

    QString result = dt.toString(locale.dateFormat(QLocale::ShortFormat));
    QString year = dt.toString("yy");

    QRegExp sepreg("[0-9]+(.)[0-9]+");

    sepreg.indexIn(result);

    return (result + sepreg.cap(1) + year);
}


QString DateTimeUtilities::shortTimeFormat(qint64 d)
{
    QString timeFormat = QLocale::system().timeFormat();
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(d);

    // Fix for a weird t at the end of the time format
    // also am/pm display
    timeFormat.replace(" t","");
    timeFormat.replace("AP","ap");

    return (dt.toString(timeFormat));
}

QString DateTimeUtilities::formattedStamp(qint64 stamp)
{
    QString date;
    qint64 now = QDateTime::currentMSecsSinceEpoch();

if (isSameDate(stamp,now))
        date = "today";
    else if (isYesterdayOf(stamp,now))
        date = "yesterday";
    else
        date = simpleDayFormat(stamp);

    date += " at " + shortTimeFormat(stamp);

    return date;
}

qint64 DateTimeUtilities::stampParser(QString stamp)
{
    // This is the format
    // 20120607T01:24:45

    QDateTime dt = QDateTime::fromString(stamp,"yyyyMMddThh:mm:ss");
    dt.setTimeSpec(Qt::UTC);
    return dt.toLocalTime().toMSecsSinceEpoch();
}
