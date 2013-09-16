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
 * official policies, either expressed or implied, of copyright holder.
 */

#ifndef CONTACTROSTER_H
#define CONTACTROSTER_H

#include "Sql/rosterdbmanager.h"

#include "contactlist.h"
#include "contact.h"
#include "group.h"

class ContactRoster : public QObject
{
    Q_OBJECT

public:
    ContactRoster(QObject *parent= 0);

    Contact& getContact(QString jid);
    bool isContactInRoster(QString jid);
    Group& getGroup(QString gjid);
    Group& getGroup(QString from, QString author, QString newSubject,
                    QString creation, QString subjectOwner, QString subjectTimestamp);
    const ContactList& getContactList() const;
    void insertContact(Contact *contact);
    void deleteContact(QString jid);
    void deleteGroup(QString gjid);
    Contact* cloneContact(Contact *c);
    int size();
    void updateAlias(Contact *contact);
    void updateName(Contact *contact);
    void updateLastSeen(Contact *contact);
    void updateContact(Contact *contact);
    void updatePhoto(Contact *contact);
    void updateStatus(Contact *contact);
    void getPhotoFromAddressBook(Contact *contact);
    void addGroupParticipant(Group *group, QString jid);
    void removeGroupParticipant(Group *group, QString jid);
    void updateSubject(Group *group);
    ContactList getBlockedJidsList();
    void updateBlock(Contact *contact);


private:
    ContactList roster;
    RosterDBManager rosterDb;

    void copy(Contact *from, Contact *to);
    Contact *createContact(QString jid);
    Group *createGroup(QString from, QString author, QString newSubject, QString creation,
                       QString subjectOwner, QString subjectTimestamp);

signals:
    void updateWholeContact(Contact *c);
    void updateAliasContact(Contact *c);
    void updateNameContact(Contact *c);
    void updateLastSeenContact(Contact *c);
    void updatePhotoContact(Contact *c);
    void updateStatusContact(Contact *c);
    void removeContact(QString jid);
    void removeGroup(QString gjid);
    void addParticipant(Group *g, Contact *c);
    void removeParticipant(Group *g, Contact *c);
    void updateSubjectGroup(Group *g);
    void updateBlockContact(Contact *c);
};

#endif // CONTACTROSTER_H
