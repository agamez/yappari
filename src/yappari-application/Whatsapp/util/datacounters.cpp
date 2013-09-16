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

#include <QSettings>

#include "datacounters.h"

#include "globalconstants.h"

DataCounters::DataCounters()
{
}

void DataCounters::increaseCounter(int counter, qint64 receivedBytes, qint64 sentBytes)
{
    received[counter] += receivedBytes;
    sent[counter] += sentBytes;

    if (counter != Messages)
    {
        received[Total] += receivedBytes;
        sent[Total] += sentBytes;
    }

#ifdef Q_WS_SCRATCHBOX
    QSettings settings(SETTINGS_ORGANIZATION,SETTINGS_COUNTERS);
    settings.setValue("received_" + QString::number(counter), received[counter]);
    settings.setValue("sent_" + QString::number(counter), sent[counter]);
    settings.setValue("received_" + QString::number(Total), received[Total]);
    settings.setValue("sent_" + QString::number(Total), sent[Total]);
#endif
}

void DataCounters::readCounters()
{
    QSettings settings(SETTINGS_ORGANIZATION,SETTINGS_COUNTERS);

    for (int i = 0; i < TOTAL_COUNTERS; i++)
    {
        received[i] = settings.value("received_" + QString::number(i), 0).toLongLong();
        sent[i] = settings.value("sent_" + QString::number(i), 0).toLongLong();
    }

}

void DataCounters::writeCounters()
{
    QSettings settings(SETTINGS_ORGANIZATION,SETTINGS_COUNTERS);

    for (int i = 0; i < TOTAL_COUNTERS; i++)
    {
        settings.setValue("received_" + QString::number(i), received[i]);
        settings.setValue("sent_" + QString::number(i), sent[i]);
    }
}

qint64 DataCounters::getReceivedBytes(int counter)
{
    return received[counter];
}

qint64 DataCounters::getSentBytes(int counter)
{
    return sent[counter];
}

void DataCounters::resetCounters()
{
    QSettings settings(SETTINGS_ORGANIZATION,SETTINGS_COUNTERS);

    for (int i = 0; i < TOTAL_COUNTERS; i++)
    {
        received[i] = 0;
        sent[i] = 0;
        settings.setValue("received_" + QString::number(i), received[i]);
        settings.setValue("sent_" + QString::number(i), sent[i]);
    }
}
