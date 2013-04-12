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
#include <QByteArray>
#include <QBuffer>

#include "globalconstants.h"
#include "rosterdbmanager.h"

#include "Contacts/group.h"
#include "Whatsapp/util/utilities.h"

#define DB_FILENAME "roster.db"
#define DB_VERSION  2

RosterDBManager::RosterDBManager(QObject *parent) :
    QObject(parent)
{
    init();
}

void RosterDBManager::init()
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

        query.exec("create table roster ("
                   "jid varchar(256) primary key,"
                   "type integer,"
                   "alias varchar(256),"
                   "from_abook boolean,"
                   "name varchar(256),"
                   "phone varchar(256) not null,"
                   "status varchar(256),"
                   "status_timestamp integer,"
                   "last_seen integer,"
                   "photo blob,"
                   "creation_timestamp integer,"
                   "author varchar(256),"
                   "subject_timestamp integer,"
                   "subject_owner varchar(256),"
                   "subject_owner_name varchar(256)"
                   ")");

        query.exec("create table settings ("
                   "version integer"
                   ")");

        query.exec("insert into settings (version) values (" +
                   QString::number(DB_VERSION) + ")");
    }
}

RosterDBManager::~RosterDBManager()
{
    db.close();
    QSqlDatabase::removeDatabase(DB_FILENAME);
}

void RosterDBManager::updateContact(Contact *c)
{
    QSqlQuery query(db);

    query.prepare("select jid from roster "
                  "where jid = :jid");
    query.bindValue(":jid",c->jid);

    query.exec();

    if (query.next() && query.value(0).toString() == c->jid)
    {
        // Update contact
        query.prepare("update roster set name=:name, type=:type, alias=:alias,"
                      "phone=:phone, status=:status, "
                      "status_timestamp=:status_timestamp, last_seen=:last_seen,"
                      "from_abook=:from_abook, photo=:photo where jid=:jid");
    }
    else
    {
        // New contact

        query.prepare("insert into roster "
                      "(jid, name, alias, type, phone, status, status_timestamp, "
                      "last_seen, from_abook, photo) "
                      "values "
                      "(:jid, :name, :alias, :type, :phone, :status, :status_timestamp, "
                      ":last_seen, :from_abook, :photo)");
    }

    query.bindValue(":jid",c->jid);
    query.bindValue(":name",c->name);
    query.bindValue(":type",c->type);
    query.bindValue(":alias",c->alias);
    query.bindValue(":from_abook",c->fromAddressBook);
    query.bindValue(":phone", (c->phone.isEmpty())
                    ? QString("+" + c->jid.left(c->jid.indexOf("@")))
                    : c->phone);
    query.bindValue(":status",c->status);
    query.bindValue(":status_timestamp",c->statusTimestamp);
    query.bindValue(":last_seen",c->lastSeen);

    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    c->photo.save(&buffer, "PNG");

    query.bindValue(":photo",bytes);

    query.exec();

    if (c->type == Contact::TypeGroup)
    {
        QSqlQuery gquery(db);
        Group *g = (Group *)c;
        gquery.prepare("update roster set creation_timestamp=:creation_timestamp, "
                      "author=:author, subject_timestamp=:subject_timestamp, "
                      "subject_owner=:subject_owner, "
                      "subject_owner_name=:subject_owner_name where jid=:jid");

        gquery.bindValue(":jid",g->jid);
        gquery.bindValue(":creation_timestamp",g->creationTimestamp);
        gquery.bindValue(":author",g->author);
        gquery.bindValue(":subject_timestamp",g->subjectTimestamp);
        gquery.bindValue(":subject_owner",g->subjectOwner);
        gquery.bindValue(":subject_owner_name",g->subjectOwnerName);

        gquery.exec();
    }

}

ContactList *RosterDBManager::getAllContacts()
{
    ContactList *list = new ContactList();

    QSqlQuery query(db);

    // Check the DB is the current version

    query.prepare("select version from settings");
    query.exec();

    if (query.next())
    {
        if (query.value(0).toInt() == 2)
        {
            qint64 startTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
            Utilities::logData("Reading roster DB...");

            query.prepare("select * from roster");
            query.exec();

            while (query.next()) {

                Contact *c;

                int type = query.value(1).toInt();
                c = ((type == Contact::TypeContact) ? new Contact() : new Group());
                c->type = (Contact::ContactType) type;
                c->jid = query.value(0).toString();
                c->alias = query.value(2).toString();
                c->fromAddressBook = query.value(3).toBool();
                c->name = query.value(4).toString();
                c->phone = query.value(5).toString();
                c->status = query.value(6).toString();
                c->statusTimestamp = query.value(7).toLongLong();
                c->lastSeen = query.value(8).toLongLong();

                QByteArray bytes = query.value(9).toByteArray();
                c->photo = QImage::fromData(bytes);

                if (c->type == Contact::TypeGroup)
                {
                    Group *g = (Group *)c;
                    g->creationTimestamp = query.value(10).toLongLong();
                    g->author = query.value(11).toString();
                    g->subjectTimestamp = query.value(12).toLongLong();
                    g->subjectOwner = query.value(13).toString();
                    g->subjectOwnerName = query.value(14).toString();
                }

                list->insert(c->jid,c);
            }
            qint64 endTime = QDateTime::currentDateTime().toMSecsSinceEpoch() - startTime;
            Utilities::logData("Roster retrieved in " + QString::number(endTime) +
                               " milliseconds.");
        }
        else if (query.value(0).toInt() == 1)
        {
            Utilities::logData("Removing old v1 roster database...");
            db.close();
            QSqlDatabase::removeDatabase(DB_FILENAME);

            QDir home = QDir::home();
            QFile file(home.path() + HOME_DIR + DB_FILENAME);

            file.remove();

            init();
        }
    }

    return list;
}

void RosterDBManager::updateAlias(Contact *c)
{
    QSqlQuery query(db);

    // Update contact
    query.prepare("update roster set alias=:alias "
                  "where jid=:jid");
    query.bindValue(":jid",c->jid);
    query.bindValue(":alias",c->alias);

    query.exec();
}

void RosterDBManager::updateName(Contact *c)
{
    QSqlQuery query(db);

    // Update contact
    query.prepare("update roster set name=:name "
                  "where jid=:jid");
    query.bindValue(":jid",c->jid);
    query.bindValue(":name",c->name);

    query.exec();
}


void RosterDBManager::removeContact(QString jid)
{
    QSqlQuery query(db);

    // Delete contact
    query.prepare("delete from roster where jid=:jid");
    query.bindValue(":jid",jid);

    query.exec();
}

void RosterDBManager::updateLastSeen(Contact *c)
{
    QSqlQuery query(db);

    Utilities::logData("Updating contact " + c->jid);

    // Update contact
    query.prepare("update roster set last_seen=:last_seen "
                  "where jid=:jid");
    query.bindValue(":jid",c->jid);
    query.bindValue(":last_seen",c->lastSeen);

    query.exec();
}
