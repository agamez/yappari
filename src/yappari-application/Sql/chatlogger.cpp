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

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

#include "globalconstants.h"
#include "chatlogger.h"

#include "Whatsapp/util/utilities.h"

#define MAX_MESSAGES               7
#define LOG_VERSION                1
#define LOG_EXTENSION              ".dblog"

ChatLogger::ChatLogger(QObject *parent):
    QObject(parent)
{
}


ChatLogger::~ChatLogger()
{
    db.close();
    QSqlDatabase::removeDatabase(jid);
}

bool ChatLogger::init(QString jid)
{
    Utilities::logData("Initializing logger for " + jid);
    this->jid = jid;

    QDir home = QDir::home();
    QFile file(home.path() + LOGS_DIR + jid.left(jid.indexOf("@")) + LOG_EXTENSION);

    bool logAvailable = file.exists();

    // Initialization of databases
    db = QSqlDatabase::addDatabase("QSQLITE",jid);

    db.setDatabaseName(file.fileName());

    if (!db.open())
     return false;

    if (!logAvailable)
    {
        QSqlQuery query(db);

        query.exec("create table log ("
                   "localid integer primary key autoincrement,"
                   "name varchar(256),"
                   "from_me boolean,"
                   "timestamp integer not null,"
                   "id varchar(20) not null,"
                   "type integer not null,"
                   "data varchar(65535),"
                   "thumb_image varchar(256),"
                   "status integer not null,"
                   "media_url varchar(256),"
                   "media_mime_type varchar(20),"
                   "media_wa_type integer,"
                   "media_size integer,"
                   "media_name varchar(256),"
                   "media_duration_seconds integer"
                   ")");

        query.exec("create table settings ("
                   "version integer"
                   ")");

        query.exec("insert into settings (version) values (" +
                   QString::number(LOG_VERSION) + ")");

        lastId = -1;
    }
    else
    {
        QSqlQuery query(db);

        query.exec("select max(localid) from log");
        query.next();
        lastId = query.value(0).toInt() + 1;
    }


    Utilities::logData("Logger for " + jid +
                       " initialized successfully.  LastId: "
                       + QString::number(lastId));
    return true;
}

void ChatLogger::logMessage(FMessage message)
{
    QSqlQuery query(db);

    query.prepare("insert into log (name, from_me, timestamp, "
                  "id, type, data, thumb_image, status, "
                  "media_url, media_mime_type, media_wa_type, media_size, "
                  "media_name, media_duration_seconds) "
                  "values (:name, :from_me, :timestamp, :id, "
                  ":type, :data, :thumb_image, :status, "
                  ":media_url, :media_mime_type, :media_wa_type, :media_size, "
                  ":media_name, :media_duration_seconds"
                  ")");

    query.bindValue(":name",message.notify_name);
    query.bindValue(":from_me",message.key.from_me);
    query.bindValue(":timestamp",message.timestamp);
    query.bindValue(":id",message.key.id);
    query.bindValue(":type",message.type);

    if (message.type == FMessage::MediaMessage)
        query.bindValue(":data",QString::fromUtf8(message.data.toBase64().constData()));
    else
        query.bindValue(":data",QString::fromUtf8(message.data));

    query.bindValue(":thumb_image",message.thumb_image);
    query.bindValue(":status",message.status);

    query.bindValue(":media_url",message.media_url);
    query.bindValue(":media_mime_type",message.media_mime_type);
    query.bindValue(":media_wa_type",message.media_wa_type);
    query.bindValue(":media_size",message.media_size);
    query.bindValue(":media_name",message.media_name);
    query.bindValue(":media_duration_seconds",message.media_duration_seconds);

    query.exec();

    Utilities::logData("logMessage(): " + query.lastError().text());
}

FMessage ChatLogger::sqlQueryResultToFMessage(QString jid,QSqlQuery& query)
{
    QString id = query.value(4).toString();
    bool from_me = query.value(2).toBool();
    FMessage msg(jid,from_me,id);

    msg.notify_name = query.value(1).toString();
    msg.setThumbImage(query.value(7).toString());
    msg.type = (FMessage::ContentType) query.value(5).toInt();
    msg.status = (FMessage::Status) query.value(8).toInt();
    msg.setTimestamp(query.value(3).toLongLong());

    msg.media_url = query.value(9).toString();
    msg.media_mime_type = query.value(10).toString();
    msg.media_wa_type = (FMessage::MediaWAType) query.value(11).toInt();
    msg.media_size = query.value(12).toLongLong();
    msg.media_duration_seconds = query.value(13).toInt();

    if (msg.type == FMessage::MediaMessage)
        msg.setData(QByteArray::fromBase64(query.value(6).toString().toUtf8()));
    else
        msg.setData(query.value(6).toString());

    return msg;
}

QList<FMessage> ChatLogger::lastMessages()
{
    QList<FMessage> list;

    // Exit if there are no more log to read
    if (lastId <= 1)
        return list;

    qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    Utilities::logData("Reading log for " + jid + "...");

    QSqlQuery query(db);

    query.prepare("select * from log "
                  "where localid < :lastid "
                  "order by localid desc limit :limit");

    query.bindValue(":lastid",lastId);
    query.bindValue(":limit",MAX_MESSAGES);
    query.exec();

    while (query.next()) {
        FMessage msg = sqlQueryResultToFMessage(jid,query);

        list.append(msg);

        lastId = query.value(0).toInt();
    }
    qint64 endTime = QDateTime::currentDateTime().toMSecsSinceEpoch() - startTime;
    Utilities::logData("Log retrieved in " + QString::number(endTime) +
                       " milliseconds.");
    return list;
}

void ChatLogger::updateLoggedMessage(FMessage message)
{
    QSqlQuery query(db);

    query.prepare("update log set status=:status where id=:id");

    query.bindValue(":id",message.key.id);
    query.bindValue(":status",message.status);

    query.exec();
}

void ChatLogger::deleteAllMessages()
{
    db.close();
    QSqlDatabase::removeDatabase(jid);

    QDir home = QDir::home();
    QFile file(home.path() + LOGS_DIR + jid.left(jid.indexOf("@")) + LOG_EXTENSION);

    file.remove();

    init(jid);
}

FMessage ChatLogger::lastMessage()
{
    QSqlQuery query(db);
    FMessage msg;

    query.prepare("select * from log "
                  "where localid = (select max(localid) from log)");
    query.exec();

    if (query.next()) {
        msg = sqlQueryResultToFMessage(jid,query);
    } else {
        QString s = "no previous message";
        msg.setData(s);
    }

    return msg;
}

FMessage ChatLogger::lastMessage(QString jid)
{
    FMessage msg;

    QString dbName = jid + "_static";
    QDir home = QDir::home();
    QFile file(home.path() + LOGS_DIR + jid.left(jid.indexOf("@")) + LOG_EXTENSION);

    if (!file.exists())
        return msg;

    // Initialization of databases
    QSqlDatabase staticDb = QSqlDatabase::addDatabase("QSQLITE",dbName);

    staticDb.setDatabaseName(file.fileName());

    if (staticDb.open())
    {
        QSqlQuery query(staticDb);
        query.prepare("select * from log "
                      "where localid = (select max(localid) from log)");
        query.exec();

        if (query.next()) {
            msg = sqlQueryResultToFMessage(jid,query);
        } else {
            QString s = "no previous message";
            msg.setData(s);
        }
    }

    QSqlDatabase::removeDatabase(dbName);

    return msg;
}

