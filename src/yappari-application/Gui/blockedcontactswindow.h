#ifndef BLOCKEDCONTACTSWINDOW_H
#define BLOCKEDCONTACTSWINDOW_H

#include "Contacts/contactroster.h"

#include "contactselectionmodel.h"
#include "groupparticipantitem.h"

#include <QMainWindow>

namespace Ui {
    class BlockedContactsWindow;
}

class BlockedContactsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BlockedContactsWindow(ContactRoster *roster, QWidget *parent = 0);
    ~BlockedContactsWindow();

public slots:
    void showBlocked();


private:
    Ui::BlockedContactsWindow *ui;
    ContactSelectionModel *model;
    ContactRoster *roster;
    ContactList blockedJids;
    QHash<QString,GroupParticipantItem *> blockedContacts;

    void addBlockedItemToModel(QString jid);
    void updateBlockedCount();

protected:
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // BLOCKEDCONTACTSWINDOW_H
