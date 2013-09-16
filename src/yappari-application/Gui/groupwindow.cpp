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

#include <QMaemo5InformationBox>
#include <QMessageBox>
#include <QMenuBar>

#include "client.h"

#include "ui_chatwindow.h"
#include "groupwindow.h"
#include "groupinfowindow.h"
#include "groupsubjectwindow.h"

#include "Whatsapp/util/utilities.h"

GroupWindow::GroupWindow(Group *group, QWidget *parent) :
    ChatWindow(group,parent)
{
    setOnlineText("Group");

    QAction *actionChangeSubject = new QAction(this);
    actionChangeSubject->setObjectName(QString::fromUtf8("actionChangeSubject"));
    actionChangeSubject->setText("Change Subject");

    QAction *actionViewGroup = new QAction(this);
    actionViewGroup->setObjectName(QString::fromUtf8("actionViewGroup"));
    actionViewGroup->setText("Group Info");

    QAction *actionLeaveGroup = new QAction(this);
    actionLeaveGroup->setObjectName(QString::fromUtf8("actionLeaveGroup"));
    actionLeaveGroup->setText("Leave Group");

    QMenu *groupMenu = menuBar()->addMenu("&Group");
    groupMenu->addAction(actionChangeSubject);
    groupMenu->addAction(actionViewGroup);
    groupMenu->addAction(actionLeaveGroup);

    ui->chatMenu->removeAction(ui->actionViewContact);

    // Actions

    connect(actionChangeSubject,SIGNAL(triggered()),
            this,SLOT(changeSubjectAction()));

    connect(actionLeaveGroup,SIGNAL(triggered()),
            this,SLOT(requestLeaveGroupAction()));

    connect(actionViewGroup,SIGNAL(triggered()),
            this,SLOT(viewGroup()));

}

void GroupWindow::changeSubjectAction()
{
    GroupSubjectWindow *groupSubjectWindow =
            new GroupSubjectWindow((Group *) contact, this);

    connect(groupSubjectWindow,SIGNAL(changeSubject(QString, QString)),
            Client::mainWin,SLOT(sendSetGroupSubjectFromChat(QString,QString)));

    groupSubjectWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
    groupSubjectWindow->setAttribute(Qt::WA_DeleteOnClose);
    groupSubjectWindow->setWindowFlags(groupSubjectWindow->windowFlags() | Qt::Window);

    groupSubjectWindow->show();
}


void GroupWindow::setGroup(Group *group, bool notify)
{
    setContact(group);

    if (notify && group->subjectOwner != Client::myJid)
        QMaemo5InformationBox::information(this,
                                           Utilities::removeEmoji(group->subjectOwnerName) +
                                           " changed the subject to " +
                                           group->name);
}

void GroupWindow::requestLeaveGroupAction()
{
    QMessageBox msg(this);

    msg.setText("Are you sure you want to leave this group?");
    msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

    if (msg.exec() == QMessageBox::Yes)
    {
        emit requestLeaveGroup(contact->jid);
        close();
    }
}


void GroupWindow::viewGroup()
{
    Group *group = (Group *) contact;

    GroupInfoWindow *groupInfoWindow = new GroupInfoWindow(group, Client::roster, this);

    emit photoRefresh(group->jid, group->photoId, false);
    emit getParticipants(group->jid);

    groupInfoWindow->setAttribute(Qt::WA_Maemo5StackedWindow);
    groupInfoWindow->setAttribute(Qt::WA_DeleteOnClose);
    groupInfoWindow->setWindowFlags(groupInfoWindow->windowFlags() | Qt::Window);

    groupInfoWindow->show();

}

void GroupWindow::groupError()
{
    QMaemo5InformationBox::information(this,"Can't send to this group, not a member");
}
