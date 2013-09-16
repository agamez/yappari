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

#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QMenu>

#include "selectcontactdialog.h"
#include "ui_selectcontactdialog.h"

#include "Contacts/contactitem.h"
#include "Contacts/contactlistiterator.h"

#include "Whatsapp/util/utilities.h"

#include "contactdisplayitem.h"
#include "contactdisplaydelegate.h"

#include "globalconstants.h"
#include "client.h"

SelectContactDialog::SelectContactDialog(ContactRoster *roster, QWidget *parent,
                                         bool contextMenuEnabled) :
    QDialog(parent),
    ui(new Ui::SelectContactDialog)
{
    ui->setupUi(this);

    /*
    QFont font = ui->searchLabel->font();
    font.setBold(true);
    ui->searchLabel->setFont(font);
    */

    this->mw = (contextMenuEnabled) ? (MainWindow *)parent : 0;

    this->roster = roster;
    model = new ContactSelectionModel(ui->listView);
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setSourceModel(model);
    ui->listView->setModel(proxy);
    ui->listView->installEventFilter(this);

    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ContactDisplayDelegate *delegate = new ContactDisplayDelegate(ui->listView);

    ui->listView->setItemDelegate(delegate);

    ContactList contactList = roster->getContactList();
    foreach(Contact *contact, contactList)
    {
        if (contact->type == Contact::TypeContact && contact->jid != Client::myJid)
            model->appendRow(new ContactDisplayItem(contact));
    }

    model->sort(0);

    connect(this,SIGNAL(sendRightButtonClicked(QPoint)),
            this,SLOT(contextMenu(QPoint)));

    connect(ui->lineEdit,SIGNAL(textChanged(QString)),
            proxy,SLOT(setFilterFixedString(QString)));

    ui->lineEdit->hide();

}

SelectContactDialog::~SelectContactDialog()
{
    delete ui;
}

Contact& SelectContactDialog::getSelectedContact()
{
    //ContactDisplayItem *item = (ContactDisplayItem *)
    //        model->item(ui->listView->currentIndex().row());


    QModelIndex index = ui->listView->currentIndex();
    QSortFilterProxyModel *proxy = (QSortFilterProxyModel *) ui->listView->model();

    Utilities::logData("INDEX DATA " + index.data(Qt::UserRole + 1).toString());

    ContactDisplayItem *item =
            (ContactDisplayItem *) model->itemFromIndex(proxy->mapToSource(index));

    return (*item->getContact());
}

void SelectContactDialog::accept()
{
    if (ui->listView->currentIndex().row() >= 0)
        QDialog::accept();
}

bool SelectContactDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu && mw != 0) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        sendRightButtonClicked(mouseEvent->globalPos());
        return true;
    }
    else if (obj != ui->lineEdit && event->type() == QEvent::KeyPress)
    {
        ui->lineEdit->show();
        ui->lineEdit->setFocus();
        ui->lineEdit->event(event);

        return true;
    }
    else
        return QDialog::eventFilter(obj, event);
}

void SelectContactDialog::contextMenu(QPoint p)
{
    Utilities::logData("Context Menu requested");

    QModelIndex index = ui->listView->indexAt(ui->listView->viewport()->mapFromGlobal(p));

    if (index.isValid())
    {
        QString jid = index.data(Qt::UserRole + 1).toString();
        Utilities::logData("Index jid: " + jid);

        QMenu *menu = new QMenu(this);
        QAction *viewContact = new QAction("View Contact",this);
        QAction *removeContact = new QAction("Remove Contact",this);
        menu->addAction(viewContact);
        menu->addAction(removeContact);

        QAction *action = menu->exec(p);
        if (action == viewContact)
        {
            Contact& c = roster->getContact(jid);
            emit showContactInfo(&c);
            close();
        }
        else if (action == removeContact)
        {
            QMessageBox msg(this);

            Contact& c = roster->getContact(jid);

            if (mw->hasChatOpen(c))
            {
                msg.setText("You have an open chat with this contact. Close it first.");
                msg.exec();
            }
            else if (Client::isSynchronizing)
            {
                msg.setText("Contacts are being synchronized right now. Please try again later");
                msg.exec();
            }
            else
            {
                QString text = (c.fromAddressBook &&
                                (Client::sync == SYNC_ENABLED | Client::sync == SYNC_INTL_ONLY))
                        ? "This contact is from your Address Book and it will reappear "
                          "in this list in the next synchronization."
                        : "This will remove this contact permanently.";

                msg.setText(text + "\n\nAre you sure you want to remove this contact?");
                msg.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);

                if (msg.exec() == QMessageBox::Yes)
                {
                    model->removeRow(index.row());
                    roster->deleteContact(jid);
                }
            }
        }
        Utilities::logData("Exit Menu");
        ui->listView->clearSelection();
    }
}
