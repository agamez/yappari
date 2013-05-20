#include "Whatsapp/util/utilities.h"

#include "groupparticipantitem.h"

GroupParticipantItem::GroupParticipantItem(Contact *c)
{

    this->contact = c;

    setEditable(false);

    setData(c->jid,Qt::UserRole + 1);

    updatePhoto();
    updateData();
}

void GroupParticipantItem::updatePhoto()
{
    if (contact->photoId.isEmpty())
        setData(QImage("/usr/share/icons/hicolor/64x64/hildon/general_default_avatar.png"), Qt::UserRole);
    else
        setData(contact->photo, Qt::UserRole);
}


void GroupParticipantItem::updateData()
{
    QString name = contact->name.isEmpty()
            ? (contact->alias.isEmpty() ? contact->phone : contact->alias)
            : contact->name;

    setData(Utilities::WATextToHtml(name, 32),Qt::DisplayRole);
}
