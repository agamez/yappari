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

#include "globalconstants.h"
#include "chatlogger.h"

#include "Whatsapp/util/utilities.h"

#define MAX_MESSAGES               7
#define LOG_VERSION                3
#define LOG_EXTENSION              ".dblog"

// Column definitions

#define LOG_LOCALID                  0
#define LOG_NAME                     1
#define LOG_FROM_ME                  2
#define LOG_TIMESTAMP                3
#define LOG_ID                       4
#define LOG_TYPE                     5
#define LOG_DATA                     6
#define LOG_THUMB_IMAGE              7
#define LOG_STATUS                   8
#define LOG_MEDIA_URL                9
#define LOG_MEDIA_MIME_TYPE         10
#define LOG_MEDIA_WA_TYPE           11
#define LOG_MEDIA_SIZE              12
#define LOG_MEDIA_NAME              13
#define LOG_MEDIA_DURATION_SECONDS  14
#define LOG_LOCAL_FILE_URI          15
#define LOG_LIVE                    16
#define LOG_LATITUDE                17
#define LOG_LONGITUDE               18

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
                   "media_duration_seconds integer,"
                   "local_file_uri varchar(512),"
                   "live boolean,"
                   "latitude real,"
                   "longitude real"
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

        // Check the DB is the current version

        query.prepare("select version from settings");
        query.exec();

        if (query.next())
        {
            int version = query.value(0).toInt();

            if (version == 1)
            {
                // Upgrade it to version 2

                Utilities::logData("Upgrading log " + jid + " to version 2");

                query.prepare("alter table log add column local_file_uri varchar(512)");
                query.exec();
                query.prepare("update settings set version=2");
                query.exec();

                version = 2;
            }

            if (version == 2)
            {
                // Upgrade it to version 3

                Utilities::logData("Upgrading log " + jid + " to version " +
                                   QString::number(LOG_VERSION));

                query.prepare("alter table log add column live boolean");
                query.exec();
                query.prepare("alter table log add column latitude real");
                query.exec();
                query.prepare("alter table log add column longitude real");
                query.exec();

                query.prepare("update settings set version="+
                              QString::number(LOG_VERSION));
                query.exec();

                version = LOG_VERSION;
            }
        }

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
                  "media_name, media_duration_seconds, local_file_uri,"
                  "live, latitude, longitude) "
                  "values (:name, :from_me, :timestamp, :id, "
                  ":type, :data, :thumb_image, :status, "
                  ":media_url, :media_mime_type, :media_wa_type, :media_size, "
                  ":media_name, :media_duration_seconds, :local_file_uri,"
                  ":live, :latitude, :longitude"
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
    query.bindValue(":local_file_uri", message.local_file_uri);

    query.bindValue(":live", message.live);
    query.bindValue(":latitude", message.latitude);
    query.bindValue(":longitude", message.longitude);

    query.exec();

    Utilities::logData("logMessage(): " + query.lastError().text());
}

FMessage ChatLogger::sqlQueryResultToFMessage(QString jid,QSqlQuery& query)
{
    QString id = query.value(LOG_ID).toString();
    bool from_me = query.value(LOG_FROM_ME).toBool();
    FMessage msg(jid,from_me,id);

    msg.notify_name = query.value(LOG_NAME).toString();
    msg.setThumbImage(query.value(LOG_THUMB_IMAGE).toString());
    msg.type = (FMessage::ContentType) query.value(LOG_TYPE).toInt();
    msg.status = (FMessage::Status) query.value(LOG_STATUS).toInt();

    if (msg.status == FMessage::Uploading)
        msg.status = FMessage::Unsent;

    msg.setTimestamp(query.value(LOG_TIMESTAMP).toLongLong());

    msg.media_url = query.value(LOG_MEDIA_URL).toString();
    msg.media_mime_type = query.value(LOG_MEDIA_MIME_TYPE).toString();
    msg.media_wa_type = (FMessage::MediaWAType) query.value(LOG_MEDIA_WA_TYPE).toInt();
    msg.media_size = query.value(LOG_MEDIA_SIZE).toLongLong();
    msg.media_name = query.value(LOG_MEDIA_NAME).toString();
    msg.media_duration_seconds = query.value(LOG_MEDIA_DURATION_SECONDS).toInt();
    msg.local_file_uri = query.value(LOG_LOCAL_FILE_URI).toString();

    msg.live = query.value(LOG_LIVE).toBool();
    msg.latitude = query.value(LOG_LATITUDE).toDouble();
    msg.longitude = query.value(LOG_LONGITUDE).toDouble();


    if (msg.type == FMessage::MediaMessage)
        msg.setData(QByteArray::fromBase64(query.value(LOG_DATA).toString().toUtf8()));
    else
        msg.setData(query.value(LOG_DATA).toString());

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

        lastId = query.value(LOG_LOCALID).toInt();
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

void ChatLogger::updateUriMessage(FMessage message)
{
    QSqlQuery query(db);

    query.prepare("update log set local_file_uri=:local_file_uri where id=:id");

    query.bindValue(":id",message.key.id);
    query.bindValue(":local_file_uri",message.local_file_uri);

    query.exec();
}


void ChatLogger::updateDuration(FMessage message)
{
    QSqlQuery query(db);

    query.prepare("update log set media_duration_seconds=:media_duration_seconds where id=:id");

    query.bindValue(":id",message.key.id);
    query.bindValue(":media_duration_seconds",message.media_duration_seconds);

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


