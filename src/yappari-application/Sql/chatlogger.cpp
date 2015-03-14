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

#include "globalconstants.h"
#include "chatlogger.h"

#include "Whatsapp/util/utilities.h"

#define MAX_MESSAGES               7
#define LOG_VERSION                7
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
#define LOG_MEDIA_CAPTION           19
#define LOG_MSG_COUNT               20
#define LOG_MSG_DELIVERED           21
#define LOG_MSG_READ                22
#define LOG_MSG_REMOTE_RESOURCE     23

#define LOG_RECEIPT_REMOTE_JID       1
#define LOG_RECEIPT_TYPE             2
#define LOG_RECEIPT_TIMESTAMP        3

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

        query.exec("BEGIN TRANSACTION");
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
                   "longitude real,"
                   "media_caption varchar(160),"
                   "msg_count integer,"
                   "msg_delivered integer,"
                   "msg_read integer,"
                   "remote_resource varchar(20)"
                   ")");

        query.exec("create table receipt ("
                   "message_id integer not null,"
                   "participant varchar(20) not null,"
                   "type integer not null,"
                   "timestamp integer not null,"
                   "unique(message_id, participant, type))");

        query.exec("create table settings ("
                   "version integer"
                   ")");

        query.exec("insert into settings (version) values (" +
                   QString::number(LOG_VERSION) + ")");

        query.exec("END TRANSACTION");
        lastId = -1;
    }
    else
    {
        QSqlQuery query(db);

        // Check the DB is the current version

        query.exec("select version from settings");

        if (query.next())
        {
            int version = query.value(0).toInt();

            query.exec("BEGIN TRANSACTION");
            switch(version)
            {
            case 1:
                // Upgrade it to version 2

                Utilities::logData("Upgrading log " + jid + " to version 2");

                query.exec("alter table log add column local_file_uri varchar(512)");

            case 2:
                // Upgrade it to version 3

                Utilities::logData("Upgrading log " + jid + " to version " +
                                   QString::number(LOG_VERSION));

                query.exec("alter table log add column live boolean");
                query.exec("alter table log add column latitude real");
                query.exec("alter table log add column longitude real");

            case 3:
                // Upgrade it to version 4
                Utilities::logData("Upgrading log " + jid + " to version " +
                                   QString::number(LOG_VERSION));

                query.exec("alter table log add column media_caption varchar(160)");

            case 4:
                // Upgrade it to version 5

                Utilities::logData("Upgrading log " + jid + " to version " +
                                   QString::number(LOG_VERSION));

                query.exec("alter table log add column msg_count integer");
                query.exec("alter table log add column msg_delivered varchar(8192)");
                query.exec("alter table log add column msg_read varchar(8192)");

            case 5:
                // Upgrade it to version 6
                Utilities::logData("Upgrading log " + jid + " to version " +
                                   QString::number(LOG_VERSION));

                // sqlite doesn't allow column type alteration, so we must do this workaround
                query.exec("alter table log rename to tmp_log");
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
                           "longitude real,"
                           "media_caption varchar(160),"
                           "msg_count integer,"
                           "msg_delivered integer,"
                           "msg_read integer"
                           ")");
                query.exec("insert into log select * from tmp_log");
                query.exec("drop table tmp_log");

            case 6:
                // Upgrade it to version 7
                Utilities::logData("Upgrading log " + jid + " to version " +
                                   QString::number(LOG_VERSION));

		query.exec("create table receipt ("
                           "message_id integer not null,"
                           "participant varchar(20) not null,"
                           "type integer not null,"
			   "timestamp integer not null,"
			   "unique(message_id, participant, type))");
                query.exec("alter table log add remote_resource varchar(20)");
            }
            query.exec("update settings set version=" +
                        QString::number(LOG_VERSION));
            query.exec("END TRANSACTION");
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

    query.exec("BEGIN TRANSACTION");
    query.prepare("insert into log (name, from_me, timestamp, "
                  "id, type, data, thumb_image, status, "
                  "media_url, media_mime_type, media_wa_type, media_size, "
                  "media_name, media_duration_seconds, local_file_uri,"
                  "live, latitude, longitude, media_caption,"
                  "msg_count, msg_delivered, msg_read, remote_resource)"
                  "values (:name, :from_me, :timestamp, :id, "
                  ":type, :data, :thumb_image, :status, "
                  ":media_url, :media_mime_type, :media_wa_type, :media_size, "
                  ":media_name, :media_duration_seconds, :local_file_uri,"
                  ":live, :latitude, :longitude, :media_caption,"
                  ":msg_count, :msg_delivered, :msg_read, :remote_resource"
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
    query.bindValue(":media_caption", message.media_caption);	

    query.bindValue(":msg_count", message.count);
    query.bindValue(":msg_delivered", message.delivered);
    query.bindValue(":msg_read", message.read);

    query.bindValue(":remote_resource", message.remote_resource);
    query.exec();

    query.exec("select max(localid) from log");
    query.next();
    int log_id = query.value(0).toInt();

    Utilities::logData("logMessage(): " + query.lastError().text());

    for(int i=0; i<message.receipts.size(); i++)
    {
        Utilities::logData("INSERTING receipt");
        query.prepare("insert into receipt (message_id, participant, type, timestamp) "
                      "values(:message_id, :participant, :type, :timestamp)");
        query.bindValue(":message_id,", log_id);
        query.bindValue(":participant", message.receipts.at(i).remote_jid);
        query.bindValue(":type", message.receipts.at(i).type);
        query.bindValue(":timestamp", message.receipts.at(i).timestamp);
        query.exec();
    }

    query.exec("END TRANSACTION");
}

int ChatLogger::getMessageReceipts(QSqlQuery& query, FMessage &msg)
{
    int receipts=0;
    int localid = query.value(LOG_LOCALID).toInt();

    query.prepare("select * from receipt "
                  "where message_id = :message_id "),
    query.bindValue(":message_id",localid);
    query.exec();

    while (query.next()) {
        Receipt r(query.value(LOG_RECEIPT_REMOTE_JID).toString(),
                  (Receipt::ReceiptType)query.value(LOG_RECEIPT_TYPE).toInt(),
                  query.value(LOG_RECEIPT_TIMESTAMP).toLongLong());
        msg.receipts.append(r);
        receipts++;
    }
    return receipts;
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
    msg.media_caption = query.value(LOG_MEDIA_CAPTION).toString();
    msg.count = query.value(LOG_MSG_COUNT).toInt();
    msg.read = query.value(LOG_MSG_READ).toInt();
    msg.delivered = query.value(LOG_MSG_DELIVERED).toInt();
    msg.remote_resource = query.value(LOG_MSG_REMOTE_RESOURCE).toString();


    if (msg.type == FMessage::MediaMessage)
        msg.setData(QByteArray::fromBase64(query.value(LOG_DATA).toString().toUtf8()));
    else
        msg.setData(query.value(LOG_DATA).toString());

    int localid = query.value(LOG_LOCALID).toInt();


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

    query.exec("BEGIN TRANSACTION");

    query.prepare("update log set status=:status, msg_count=:msg_count, "
                "msg_delivered=:msg_delivered, msg_read=:msg_read where id=:id");

    query.bindValue(":id",message.key.id);
    query.bindValue(":status",message.status);
    query.bindValue(":msg_count",message.count);
    query.bindValue(":msg_delivered",message.delivered);
    query.bindValue(":msg_read",message.read);

    query.exec();

    Utilities::logData("logMessage(): " + query.lastError().text());

    query.prepare("select localid from log where id=:id");
    query.bindValue(":id",message.key.id);
    query.exec();

    query.next();
    int log_id = query.value(0).toInt();

    for(int i=0; i<message.receipts.size(); i++)
    {
        Utilities::logData("UPDATING receipts");
        query.prepare("insert or replace into receipt (message_id, participant, type, timestamp) "
                      "values(:message_id, :participant, :type, :timestamp)");
        query.bindValue(":message_id,", log_id);
        query.bindValue(":participant", message.receipts.at(i).remote_jid);
        query.bindValue(":type", message.receipts.at(i).type);
        query.bindValue(":timestamp", message.receipts.at(i).timestamp);
        query.exec();
    }

    Utilities::logData("logMessage(): " + query.lastError().text());

    query.exec("END TRANSACTION");
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

void ChatLogger::deleteMessage(FMessage message)
{
    QSqlQuery query(db);

    query.prepare("delete from log where id=:id");

    query.bindValue(":id",message.key.id);
    query.exec();
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
        getMessageReceipts(query, msg);
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
            getMessageReceipts(query, msg);
        } else {
            QString s = "no previous message";
            msg.setData(s);
        }
    }

    QSqlDatabase::removeDatabase(dbName);

    return msg;
}
