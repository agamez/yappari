#include "blockedcontactswindow.h"
#include "ui_blockedcontactswindow.h"

#include <QMessageBox>
#include <QMaemo5InformationBox>
#include <QMaemo5Style>
#include <QMenu>
#include <QMouseEvent>

#include "groupparticipantdelegate.h"

#include "client.h"

BlockedContactsWindow::BlockedContactsWindow(ContactRoster *roster, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BlockedContactsWindow)
{
    ui->setupUi(this);

    model = new ContactSelectionModel(ui->listView);
    ui->listView->setModel(model);
    ui->listView->installEventFilter(this);

    GroupParticipantDelegate *delegate = new GroupParticipantDelegate(ui->listView);

    ui->listView->setItemDelegate(delegate);

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");
    ui->labelBlocked->setStyleSheet("font-size:18px;color:" + color.name());

    this->roster = roster;

    showBlocked();

    updateBlockedCount();
}

BlockedContactsWindow::~BlockedContactsWindow()
{
    delete ui;
}

void BlockedContactsWindow::updateBlockedCount()
{
    ui->labelBlocked->setText("Blocked contacts: " + QString::number(blockedContacts.size()));
}


void BlockedContactsWindow::addBlockedItemToModel(QString jid)
{
    if (!blockedContacts.contains(jid))
    {
        Contact &c = roster->getContact(jid);

        GroupParticipantItem *item = new GroupParticipantItem(&c);
        model->appendRow(item);
        blockedContacts.insert(c.jid,item);
    }
}

void BlockedContactsWindow::showBlocked()
{
    this->blockedJids = roster->getBlockedJidsList();

    foreach (QString jid, blockedJids.keys())
        addBlockedItemToModel(jid);

    model->sort(0);
}

bool BlockedContactsWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*> (event);
        QPoint p = mouseEvent->globalPos();

        QModelIndex index = ui->listView->indexAt(
                    ui->listView->viewport()->mapFromGlobal(p));

        if (index.isValid())
        {
            QString jid = index.data(Qt::UserRole + 1).toString();

            if (jid != Client::myJid)
            {
                QMenu *menu = new QMenu(this);
                QAction *unblockContact = new QAction("Unblock Contact",this);
                QAction *viewContact = new QAction("View Contact",this);

                menu->addAction(viewContact);
                menu->addAction(unblockContact);

                QAction *action = menu->exec(p);

                if (action == unblockContact)
                {
                    if (Client::connectionStatus != Client::LoggedIn)
                    {
                        QMaemo5InformationBox::information(this,"You have to be logged in to unblock a contact",
                                                           QMaemo5InformationBox::NoTimeout);
                    }
                    else
                    {
                        GroupParticipantItem *item = blockedContacts.value(jid);
                        model->removeRow(item->row());
                        blockedContacts.remove(jid);

                        Client::mainWin->blockOrUnblockContact(jid, false);
                        updateBlockedCount();
                    }
                }

                else if (action == viewContact)
                {
                    Contact& c = roster->getContact(jid);

                    ContactInfoWindow *window = new ContactInfoWindow(&c,this);

                    Client::mainWin->requestContactStatus(c.jid);
                    Client::mainWin->requestPhotoRefresh(c.jid, c.photoId, false);

                    window->setAttribute(Qt::WA_Maemo5StackedWindow);
                    window->setAttribute(Qt::WA_DeleteOnClose);
                    window->setWindowFlags(window->windowFlags() | Qt::Window);
                    window->show();
                }
            }

            ui->listView->clearSelection();
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
