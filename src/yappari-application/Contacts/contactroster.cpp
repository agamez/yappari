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

#include "contactroster.h"

#include <QContactPhoneNumber>
#include <QContactDetailFilter>
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
    connect(this,SIGNAL(updateContact(Contact *)),&rosterDb,SLOT(updateContact(Contact *)));
    connect(this,SIGNAL(updateAliasContact(Contact *)),&rosterDb,SLOT(updateAlias(Contact *)));
    connect(this,SIGNAL(updateNameContact(Contact *)),&rosterDb,SLOT(updateName(Contact *)));
    connect(this,SIGNAL(updateLastSeenContact(Contact *)),&rosterDb,SLOT(updateLastSeen(Contact *)));
    connect(this,SIGNAL(removeContact(QString)),&rosterDb,SLOT(removeContact(QString)));


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
    roster.insert(jid,c);
    emit updateContact(c);
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
        emit updateContact(c);
    }

    return *c;
}

Group& ContactRoster::getGroup(QString from, QString author, QString newSubject,
                               QString creation)
{
    return getGroup(from,author,newSubject,creation,
                    from,creation);
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
        Utilities::logData("Group found: " + from);
        c->author = author;
        c->creationTimestamp = creation.toLongLong();
        c->name = newSubject;
        c->subjectOwner = subjectOwner;
        c->subjectTimestamp = subjectTimestamp.toLongLong();
        emit updateContact(c);
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
    emit updateContact(c);
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
        c = new Contact();
        copy(contact,c);
        roster.insert(c->jid,c);
    }
    else
    {
        copy(contact,c);
    }

    emit updateContact(c);
}

int ContactRoster::size()
{
    return roster.size();
}

void ContactRoster::deleteContact(QString jid)
{
    roster.remove(jid);
    emit removeContact(jid);
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






