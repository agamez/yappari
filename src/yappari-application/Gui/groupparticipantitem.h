#ifndef GROUPPARTICIPANTITEM_H
#define GROUPPARTICIPANTITEM_H

#include <QStandardItem>

#include "Contacts/contact.h"

class GroupParticipantItem : public QStandardItem
{
public:
    GroupParticipantItem(Contact *c);

private:
    Contact *contact;

    void updatePhoto();
    void updateData();
};

#endif // GROUPPARTICIPANTITEM_H
