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
#include <QMouseEvent>
#include <QMenu>

#include "groupparticipantdelegate.h"
#include "selectcontactdialog.h"

#include "selectgroupparticipantswindow.h"
#include "ui_selectgroupparticipantswindow.h"

SelectGroupParticipantsWindow::SelectGroupParticipantsWindow(ContactRoster *roster,
                                                             QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SelectGroupParticipantsWindow)
{
    this->roster = roster;

    ui->setupUi(this);

    model = new ContactSelectionModel(ui->listView);
    ui->listView->setModel(model);
    ui->listView->installEventFilter(this);

    GroupParticipantDelegate *delegate = new GroupParticipantDelegate(ui->listView);

    ui->listView->setItemDelegate(delegate);

    connect(ui->addParticipantButton,SIGNAL(pressed()),
            this,SLOT(addParticipant()));

    connect(ui->createGroupButton,SIGNAL(pressed()),
            this,SLOT(verifyGroup()));
}

SelectGroupParticipantsWindow::~SelectGroupParticipantsWindow()
{
    delete ui;
}

bool SelectGroupParticipantsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        QPoint p = mouseEvent->globalPos();

        QModelIndex index = ui->listView->indexAt(
                    ui->listView->viewport()->mapFromGlobal(p));

        if (index.isValid())
        {
            QString jid = index.data(Qt::UserRole + 1).toString();

            QMenu *menu = new QMenu(this);
            QAction *removeContact = new QAction("Remove Contact",this);
            menu->addAction(removeContact);

            QAction *action = menu->exec(p);

            if (action == removeContact)
            {
                QMessageBox msg(this);

                msg.setText("Are you sure you want to remove this contact from this group?");
                msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

                if (msg.exec() == QMessageBox::Yes)
                {
                    model->removeRow(index.row());
                    participants.remove(jid);
                    ui->listView->clearSelection();
                }

                return true;
            }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}


void SelectGroupParticipantsWindow::addParticipant()
{
    if (!roster->size())
    {
        QMaemo5InformationBox::information(this,"Contacts haven't been synchronized yet.",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else if (participants.size() == 50)
    {
        QMaemo5InformationBox::information(this,"The maximum number of participants have been reached.",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        SelectContactDialog selectContactDialog(roster,this,false);

        //connect(&selectContactDialog,SIGNAL(showContactInfo(Contact*)),
        //        this,SLOT(viewContact(Contact*)));

        if (selectContactDialog.exec() == QDialog::Accepted)
        {
            Contact& contact = selectContactDialog.getSelectedContact();

            // Add it only if it hasn't been added before
            if (!participants.contains(contact.jid))
            {
                GroupParticipantItem *item = new GroupParticipantItem(&contact);
                model->appendRow(item);
                participants.insert(contact.jid,item);
            }
        }
    }
}

void SelectGroupParticipantsWindow::verifyGroup()
{
    if (participants.size() == 0)
    {
        QMaemo5InformationBox::information(this,"At least one contact must be selected.",
                                           QMaemo5InformationBox::NoTimeout);
    }
    else
    {
        QStringList groupParticipants = participants.keys();
        emit createGroup(groupParticipants);
        close();
    }
}
