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

#ifndef ROSTERDBMANAGER_H
#define ROSTERDBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QList>
#include <QObject>

#include "Contacts/group.h"
#include "Contacts/contact.h"
#include "Contacts/contactlist.h"

class RosterDBManager : public QObject
{
    Q_OBJECT
public:
    explicit RosterDBManager(QObject *parent = 0);
    ~RosterDBManager();

    ContactList *getAllContacts();

signals:

public slots:
    void updateContact(Contact *c);
    void updateAlias(Contact *c);
    void updateName(Contact *c);
    void updateLastSeen(Contact *c);
    void updatePhoto(Contact *c);
    void updateStatus(Contact *c);
    void removeContact(QString jid);
    void removeGroup(QString gjid);
    void addParticipant(Group *g, Contact *c);
    void removeParticipant(Group *g, Contact *c);
    void updateSubjectGroup(Group *g);
    void updateBlockContact(Contact *c);



private:
    QSqlDatabase db;

    void init();

};

#endif // ROSTERDBMANAGER_H
