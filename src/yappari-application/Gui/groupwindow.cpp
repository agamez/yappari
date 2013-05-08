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

#include <QMaemo5InformationBox>
#include <QMenuBar>

#include "client.h"

#include "ui_chatwindow.h"
#include "groupwindow.h"
#include "groupsubjectdialog.h"

#include "Whatsapp/util/utilities.h"

GroupWindow::GroupWindow(Group group, QWidget *parent) :
    ChatWindow(group,parent)
{
    setOnlineText("Group");

    QAction *actionChangeSubject = new QAction(this);
    actionChangeSubject->setObjectName(QString::fromUtf8("actionChangeSubject"));
    actionChangeSubject->setText("Change Subject");

    QAction *actionLeaveGroup = new QAction(this);
    actionLeaveGroup->setObjectName(QString::fromUtf8("actionLeaveGroup"));
    actionLeaveGroup->setText("Leave Group");

    QMenu *groupMenu = menuBar()->addMenu("&Group");
    groupMenu->addAction(actionChangeSubject);
    groupMenu->addAction(actionLeaveGroup);

    // ui->actionViewContact->setText("View Group");
    ui->chatMenu->removeAction(ui->actionViewContact);

    // Actions

    connect(actionChangeSubject,SIGNAL(triggered()),
            this,SLOT(changeSubjectAction()));

    connect(actionLeaveGroup,SIGNAL(triggered()),
            this,SLOT(requestLeaveGroupAction()));

}

void GroupWindow::changeSubjectAction()
{
    GroupSubjectDialog dialog(contact.name,this);

    if (dialog.exec() == QDialog::Accepted)
    {
        QString newSubject = dialog.getSubject();
        Utilities::logData("Subject change to: " + newSubject);
        emit changeSubject(contact.jid,newSubject);
    }
}

void GroupWindow::setGroup(Group &group, bool notify)
{
    setContact(group);

    if (notify && group.subjectOwner != Client::myJid)
        QMaemo5InformationBox::information(this,group.subjectOwnerName + " changed the subject to " +
                                           group.name);
}

void GroupWindow::requestLeaveGroupAction()
{
    emit requestLeaveGroup(contact.jid);
}
