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

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QList>

#include "conversationsdb.h"
#include "globalconstants.h"

#include "Whatsapp/util/utilities.h"

#define DB_FILENAME "chats.db"
#define DB_VERSION  1

ConversationsDB::ConversationsDB(QObject *parent):
    QObject(parent)
{
    QDir home = QDir::home();
    QFile file(home.path() + HOME_DIR + DB_FILENAME);

    bool dbAvailable = file.exists();

    // Initialization of databases
    db = QSqlDatabase::addDatabase("QSQLITE",DB_FILENAME);

    db.setDatabaseName(file.fileName());

    if (!db.open())
        return;

    if (!dbAvailable)
    {
        QSqlQuery query(db);

        query.exec("create table chats ("
                   "jid varchar(256) primary key,"
                   "muted boolean,"
                   "mute_timestamp integer"
                   ")");

        query.exec("create table settings ("
                   "version integer"
                   ")");

        query.exec("insert into settings (version) values (" +
                   QString::number(DB_VERSION) + ")");
    }
}

void ConversationsDB::createOrUpdateChat(ConversationsDBEntry entry)
{
    QSqlQuery query(db);

    query.prepare("select jid from chats "
                  "where jid = :jid");

    query.bindValue(":jid",entry.jid);

    query.exec();

    if (!(query.next() && query.value(0).toString() == entry.jid))
    {
        // New chat

        query.prepare("insert into chats "
                      "(jid, muted, mute_timestamp) "
                      "values "
                      "(:jid, :muted, :mute_timestamp)");
    }
    else
    {
        query.prepare("update chats set muted=:muted,"
                      "mute_timestamp=:mute_timestamp "
                      "where jid=:jid");

    }

    query.bindValue(":jid",entry.jid);
    query.bindValue(":muted",entry.muted);
    query.bindValue(":mute_timestamp",entry.muteExpireTimestamp);

    query.exec();
}

void ConversationsDB::createChat(ConversationsDBEntry entry)
{
    createOrUpdateChat(entry);
}

void ConversationsDB::updateChat(ConversationsDBEntry entry)
{
    createOrUpdateChat(entry);
}

QList<ConversationsDBEntry> ConversationsDB::getAllChats()
{
    QList<ConversationsDBEntry> list;

    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    Utilities::logData("Reading open chats DB...");

    QSqlQuery query(db);
    query.prepare("select * from chats");
    query.exec();

    while (query.next()) {

        ConversationsDBEntry entry;

        entry.jid = query.value(0).toString();
        entry.muted = query.value(1).toBool();
        entry.muteExpireTimestamp = query.value(2).toLongLong();

        list.append(entry);
    }
    qint64 endTime = QDateTime::currentDateTime().toMSecsSinceEpoch() - startTime;
    Utilities::logData("Open Chats retrieved in " + QString::number(endTime) +
                       " milliseconds.");

    return list;
}

void ConversationsDB::removeChat(QString jid)
{
    QSqlQuery query(db);
    query.prepare("delete from chats where jid=:jid");
    query.bindValue(":jid",jid);
    query.exec();
}


