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

#include <QSslConfiguration>
#include <QRegExp>
#include <QImage>
#include <QUrl>

#include "Dbus/gtkbindings-new.h"
#include <libosso.h>
#include <libebook/e-book.h>
#include <libosso-abook/osso-abook.h>
#include <gdk/gdkpixbuf.h>

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/qtmd5digest.h"

#include "qt-json/json.h"

#include "client.h"
#include "version.h"
#include "globalconstants.h"
#include "contactsyncer.h"

ContactSyncer::ContactSyncer(ContactRoster* roster,
                             QObject *parent)
    : QObject(parent)
{
    isSyncing = false;

    this->roster = roster;
}

void ContactSyncer::freeAddressBook()
{
    Utilities::logData("syncer: freeing address book");
    QStringList list = abook.keys();

    foreach (QString key, list)
    {
        Contact *c = abook.value(key);
        delete c;
    }

    abook.clear();
    totalPhones = currentPhone = 0;
}

void ContactSyncer::getAddressBook()
{
    Utilities::logData("Retrieving address book...");
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

    GError *error = NULL;

    OssoABookRoster *o_roster = osso_abook_aggregator_get_default (&error);
    if (error)
    {
        QString errorStr = error->message;
        Utilities::logData("getAddressBook(): " + errorStr);
        g_error_free(error);
    }

    OssoABookAggregator *aggregator = OSSO_ABOOK_AGGREGATOR(o_roster);
    osso_abook_waitable_run(OSSO_ABOOK_WAITABLE(aggregator),
                            g_main_context_default(), &error);
    if (error)
    {
        QString errorStr = error->message;
        Utilities::logData("getAddressBook(): " + errorStr);
        g_error_free(error);
    }

    GList *contacts = osso_abook_aggregator_list_master_contacts(aggregator);


    for (GList *l = contacts;l ; l = l->next)
    {
        OssoABookContact *contact = OSSO_ABOOK_CONTACT(l->data);
        QString displayName = QString::fromUtf8(osso_abook_contact_get_display_name(contact));

        EContactField field = e_contact_field_id("mobile-phone");
        GList *attrs = e_contact_get_attributes(E_CONTACT(contact),field);
        for (GList *la = attrs; la; la = la->next)
        {
            EVCardAttribute *attr = (EVCardAttribute *) la->data;

            gchar *str = e_vcard_attribute_get_value(attr);
            QString phoneNumber = QString::fromUtf8(str);
            g_free(str);

            // ToDo: Delete everything that's not a number

            if (Client::sync.isEmpty() || Client::sync == SYNC_ENABLED ||
                    (Client::sync == SYNC_INTL_ONLY && phoneNumber.left(1) == "+"))
            {
                Contact *c = new Contact();
                c->name = displayName;
                c->phone = phoneNumber;
                c->type = Contact::TypeContact;
                // c->photo = image;
                c->fromAddressBook = true;
                abook.insert(c->phone,c);
            }
        }
        g_list_foreach(attrs, (GFunc) e_vcard_attribute_free, NULL);
        g_list_free(attrs);

    }

    g_list_free(contacts);

    qint64 endTime = QDateTime::currentMSecsSinceEpoch() - startTime;
    Utilities::logData("Address book retrieved in " + QString::number(endTime) +
                       " milliseconds.");
}

void ContactSyncer::syncContacts()
{
    if (!isSyncing && Client::connectionStatus == Client::LoggedIn)
    {
        Utilities::logData("syncer: Synchronizing contacts...");

        isSyncing = true;
        emit progress(0);

        freeAddressBook();

        ContactList list = roster->getContactList();
        foreach (Contact *c, list.values())
        {
            if (c->type == Contact::TypeContact && c->jid != Client::myJid)
            {
                if (!c->fromAddressBook)
                {
                    // If it's a temporal contact include it to the
                    // address book to synchronize it.
                    // We need to create a new one because this
                    // is gonna be freed later
                    if (!abook.contains(c->jid))
                    {
                        Contact *cc = roster->cloneContact(c);
                        abook.insert(cc->phone,cc);
                    }
                }
            }
        }

        // Fill the abook with the phone's address book
        getAddressBook();

        // Sync the abook
        syncAddressBook();
    }
}

void ContactSyncer::syncContact(Contact *c)
{
    if (!isSyncing && Client::connectionStatus == Client::LoggedIn)
    {
        Utilities::logData("syncer: Synchronizing contact: " + c->jid);

        isSyncing = true;

        // Only include a copy of the contact in the abook
        // It is gonna be freed later
        Contact *cc = roster->cloneContact(c);
        abook.clear();
        abook.insert(cc->phone,cc);

        syncAddressBook();
    }
}

void ContactSyncer::syncAddressBook()
{
    totalPhones = currentPhone = 0;

    QStringList numbers;

    if (abook.size() > 0)
    {
        foreach(Contact *c, abook.values())
            if (c->type == Contact::TypeContact && c->jid != Client::myJid)
            {
                totalPhones++;
                numbers << c->phone;
            }

        emit phoneListReady(numbers);
    }
}

void ContactSyncer::syncPhone(QString jid, QString phone)
{
    Contact *c = abook.value(phone);

    Contact *contact;

    if (c)
    {
        bool exists = roster->isContactInRoster(jid);
        bool updated = false;

        if (exists)
        {
            Contact &d = roster->getContact(jid);
            contact = &d;

            // Check if the name was changed in the address book
            if (contact->name != c->name)
            {
                updated = true;
                contact->name = c->name;
            }

            // Check if this contact wasn't in the address book
            // before but now it is
            if (contact->fromAddressBook != c->fromAddressBook)
            {
                updated = true;
                contact->fromAddressBook = c->fromAddressBook;
            }
        }
        else
        {
            contact = new Contact();
            contact->fromAddressBook = true;
            contact->name = c->name;
            contact->phone = c->phone;
            contact->jid = jid;
            contact->photoId = QString();
        }

        if (exists && updated)
            roster->updateContact(contact);
        else if (!exists)
            roster->insertContact(contact);

        // emit updateStatus(jid, contact->statusTimestamp);
        // emit updatePhoto(jid, contact->photoId, false);

        /*
        Utilities::logData("Synchronized contact:\n"
                           "Name: " + c->name +
                           "\nPhone: " + c->phone +
                           "\njid: " + c->jid +
                           "\nStatus: " + c->status);
        */

        //Utilities::logData("syncPhone(): " + jid + " " + QString::number(currentPhone)
        //                    + " " + QString::number(totalPhones));

        if (++currentPhone < totalPhones)
        {
            //if (currentPhone % ((int)(totalPhones / 10)) == 0)
            //    emit progress((int)(((currentPhone*50) / totalPhones));
        }
        else
            syncStatusAndPhotos();
    }
}

void ContactSyncer::deletePhone(QString jid, QString phone)
{
    if (roster->isContactInRoster(jid))
        roster->deleteContact(jid);

    if (abook.contains(phone))
    {
        Contact *c = abook.value(phone);
        abook.remove(phone);
        delete c;
    }

    // Utilities::logData("deletePhone(): " + jid + " " + QString::number(currentPhone)
    //                    + " " + QString::number(totalPhones));

    if (++currentPhone < totalPhones)
    {
        //if (currentPhone % ((int)(totalPhones / 10)) == 0)
        //    emit progress((int)((currentPhone*50) / totalPhones));
    }
    else
        syncStatusAndPhotos();
}

void ContactSyncer::syncStatusAndPhotos()
{
    totalPhones = roster->size();
    currentPhone = 0;

    // Utilities::logData("syncStatusAndPhotos(): " + QString::number(currentPhone)
    //                     + " " + QString::number(totalPhones));

    QTimer::singleShot(1000,this,SLOT(syncNextChunk()));
}

void ContactSyncer::syncNextChunk()
{
    QStringList jids;

    QList<Contact *> list = roster->getContactList().values();

    if (currentPhone >= totalPhones)
        finishSync();
    else
    {
        emit progress((int)((currentPhone*50) / totalPhones) + 50);

        int max = currentPhone + 5;
        while (currentPhone < max && currentPhone < totalPhones)
        {
            Contact *c = list.at(currentPhone++);
            // Utilities::logData("syncNextChunk(): " + c->jid);
            if (/*c->type == Contact::TypeContact && */!c->jid.isEmpty() && c->jid != Client::myJid)
            {
                jids << c->jid;

                emit updatePhoto(c->jid, c->photoId, false);
            }
        }
        Utilities::logData("syncNextChunk(): Requesting status of " + jids.join(" "));

        emit statusListReady(jids);

        QTimer::singleShot(1000,this,SLOT(syncNextChunk()));
    }
}


void ContactSyncer::finishSync()
{
    freeAddressBook();
    isSyncing = false;

    emit syncFinished();
}

bool ContactSyncer::isSynchronizing()
{
    return isSyncing;
}
