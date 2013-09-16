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

#include "contactroster.h"

#include <QMessageBox>
#include <QDateTime>

#include "Dbus/gtkbindings-new.h"
#include <libosso.h>
#include <libebook/e-book.h>
#include <libosso-abook/osso-abook.h>

#include "Whatsapp/util/utilities.h"

#include "contactlistiterator.h"

#include "version.h"
#include "globalconstants.h"

ContactRoster::ContactRoster(QObject *parent) : QObject(parent)
{
    connect(this,SIGNAL(updateWholeContact(Contact *)),&rosterDb,SLOT(updateContact(Contact *)));
    connect(this,SIGNAL(updateAliasContact(Contact *)),&rosterDb,SLOT(updateAlias(Contact *)));
    connect(this,SIGNAL(updateNameContact(Contact *)),&rosterDb,SLOT(updateName(Contact *)));
    connect(this,SIGNAL(updateLastSeenContact(Contact *)),&rosterDb,SLOT(updateLastSeen(Contact *)));
    connect(this,SIGNAL(updatePhotoContact(Contact *)),&rosterDb,SLOT(updatePhoto(Contact *)));
    connect(this,SIGNAL(updateStatusContact(Contact *)),&rosterDb,SLOT(updateStatus(Contact *)));
    connect(this,SIGNAL(removeContact(QString)),&rosterDb,SLOT(removeContact(QString)));
    connect(this,SIGNAL(removeGroup(QString)),&rosterDb,SLOT(removeGroup(QString)));
    connect(this,SIGNAL(addParticipant(Group*,Contact*)),&rosterDb,SLOT(addParticipant(Group*,Contact*)));
    connect(this,SIGNAL(removeParticipant(Group*,Contact*)),&rosterDb,SLOT(removeParticipant(Group*,Contact*)));
    connect(this,SIGNAL(updateSubjectGroup(Group*)),&rosterDb,SLOT(updateSubjectGroup(Group*)));
    connect(this,SIGNAL(updateBlockContact(Contact *)),&rosterDb,SLOT(updateBlockContact(Contact *)));

    ContactList *list = rosterDb.getAllContacts();

    ContactListIterator i(*list);

    while(i.hasNext())
    {
        i.next();

        Contact *c = i.value();

        roster.insert(c->jid,c);
    }

    delete list;
}

Contact& ContactRoster::getContact(QString jid)
{
    Contact *c = roster.value(jid,0);

    if (c == 0)
        c = createContact(jid);

    return *c;
}

const ContactList& ContactRoster::getContactList() const
{
    return roster;
}

bool ContactRoster::isContactInRoster(QString jid)
{
    return roster.contains(jid);
}

Contact* ContactRoster::createContact(QString jid)
{
    Contact *c = new Contact();
    c->fromAddressBook = false;
    c->phone = "+" + jid.left(jid.indexOf("@"));
    c->jid = jid;
    c->type = Contact::TypeContact;
    c->photoId = c->name = c->alias = QString();
    roster.insert(jid,c);
    updateContact(c);
    return c;
}

Group& ContactRoster::getGroup(QString gjid)
{
    Group *c = (Group *)roster.value(gjid,0);

    if (c == 0)
    {
        c = new Group();
        c->type = Contact::TypeGroup;
        c->jid = gjid;
        roster.insert(gjid,c);
        updateContact(c);
    }

    return *c;
}

Group& ContactRoster::getGroup(QString from, QString author, QString newSubject,
                               QString creation, QString subjectOwner,
                               QString subjectTimestamp)
{
    Group *c = (Group *)roster.value(from,0);

    if (c == 0)
        c = createGroup(from, author, newSubject, creation, subjectOwner, subjectTimestamp);
    else
    {
        Utilities::logData("Group found: " + from + " - Subject: " + newSubject);
        c->author = author;
        c->creationTimestamp = creation.toLongLong();
        c->name = newSubject;
        c->subjectOwner = subjectOwner;
        c->subjectTimestamp = subjectTimestamp.toLongLong();
        updateContact(c);
    }

    return *c;
}

Group* ContactRoster::createGroup(QString from, QString author, QString newSubject,
                                  QString creation, QString subjectOwner,
                                  QString subjectTimestamp)
{
    Utilities::logData("New Group: " + from);

    Group *c = new Group();
    c->type = Contact::TypeGroup;
    c->name = newSubject;
    c->jid = from;
    c->author = author;
    c->fromAddressBook = false;
    c->subjectOwner = subjectOwner;
    c->subjectTimestamp = subjectTimestamp.toLongLong();
    c->creationTimestamp = creation.toLongLong();
    roster.insert(from,c);
    updateContact(c);
    return c;
}

void ContactRoster::copy(Contact *from, Contact *to)
{
    to->phone = from->phone;
    to->jid = from->jid;
    to->status = from->status;
    to->statusTimestamp = from->statusTimestamp;
    to->name = from->name;
    to->alias = from->alias;
    to->type = from->type;
    to->fromAddressBook = from->fromAddressBook;
    to->photo = from->photo;

    to->lastSeen = from->lastSeen;
    to->typingStatus = from->typingStatus;
    to->isOnline = from->isOnline;
}

void ContactRoster::insertContact(Contact *contact)
{
    Contact *c = roster.value(contact->jid,0);

    if (c == 0)
    {
        roster.insert(contact->jid,contact);
        emit updateWholeContact(contact);
    }
}

int ContactRoster::size()
{
    return roster.size();
}

void ContactRoster::deleteContact(QString jid)
{
    if (roster.contains(jid))
    {
        Contact *c = roster.value(jid);

        roster.remove(jid);
        emit removeContact(jid);

        delete c;
    }
}

void ContactRoster::deleteGroup(QString gjid)
{
    if (roster.contains(gjid))
    {
        Group *g = (Group *) roster.value(gjid);

        roster.remove(gjid);
        emit removeGroup(gjid);

        delete g;
    }
}

void ContactRoster::updateAlias(Contact *c)
{
    emit updateAliasContact(c);
}


void ContactRoster::updateName(Contact *c)
{
    emit updateNameContact(c);
}

void ContactRoster::updateLastSeen(Contact *c)
{
    emit updateLastSeenContact(c);
}

Contact *ContactRoster::cloneContact(Contact *c)
{
    Contact *newc = new Contact();
    copy(c,newc);
    return newc;
}

void ContactRoster::updateContact(Contact *c)
{
    emit updateWholeContact(c);
}

void ContactRoster::updatePhoto(Contact *c)
{
    emit updatePhotoContact(c);
}

void ContactRoster::getPhotoFromAddressBook(Contact *c)
{
    bool hadPreviousPhoto = !c->photoId.isEmpty();

    GError *error = NULL;

    OssoABookRoster *o_roster = osso_abook_aggregator_get_default (&error);
    if (error)
    {
        QString errorStr = error->message;
        Utilities::logData("getPhotoFromAddressBook(): " + errorStr);
        g_error_free(error);
    }

    OssoABookAggregator *aggregator = OSSO_ABOOK_AGGREGATOR(o_roster);
    osso_abook_waitable_run(OSSO_ABOOK_WAITABLE(aggregator),
                            g_main_context_default(), &error);
    if (error)
    {
        QString errorStr = error->message;
        Utilities::logData("getPhotoFromAddressBook(): " + errorStr);
        g_error_free(error);
    }

    GList *list =
            osso_abook_aggregator_find_contacts_for_phone_number(aggregator,
                                                                 c->phone.toUtf8().constData(),
                                                                 FALSE);
    if (list)
    {
        OssoABookContact *contact = OSSO_ABOOK_CONTACT(list->data);
        GdkPixbuf *pixbuf = osso_abook_contact_get_photo(contact);

        if (pixbuf)
        {
            gchar *buffer;
            gsize buffer_size;
            gdk_pixbuf_save_to_bufferv(pixbuf,&buffer,&buffer_size,"png",NULL,NULL,&error);

            c->photo.loadFromData((const uchar*)buffer, buffer_size);
            c->photoId = "abook";

            // Force update in DB
            hadPreviousPhoto = true;

            g_free(buffer);

            gdk_pixbuf_unref(pixbuf);

        }
        else if (hadPreviousPhoto)
        {
            c->photo = QImage();
            c->photoId.clear();
        }

        g_list_free(list);
    }
    else if (hadPreviousPhoto)
    {
        c->photo = QImage();
        c->photoId.clear();
    }

    if (hadPreviousPhoto)
        emit updatePhotoContact(c);
}

void ContactRoster::updateStatus(Contact *contact)
{
    emit updateStatusContact(contact);
}

void ContactRoster::addGroupParticipant(Group *group, QString jid)
{
    if (group->addParticipant(jid))
    {
        Contact &c = getContact(jid);
        emit addParticipant(group,&c);
    }
}

void ContactRoster::updateSubject(Group *group)
{
    emit updateSubjectGroup(group);
}

void ContactRoster::removeGroupParticipant(Group *group, QString jid)
{
    if (group->removeParticipant(jid))
    {
        Contact &c = getContact(jid);
        emit removeParticipant(group,&c);
    }
}

ContactList ContactRoster::getBlockedJidsList()
{
    ContactList result;

    foreach (Contact *c, roster)
        if (c->blocked)
            result.insert(c->jid,c);

    return result;
}

void ContactRoster::updateBlock(Contact *contact)
{
    emit updateBlockContact(contact);
}

