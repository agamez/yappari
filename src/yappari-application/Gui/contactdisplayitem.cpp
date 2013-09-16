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

#include <QAbstractTextDocumentLayout>
#include <QTextDocument>
#include <QMaemo5Style>

#include "contactdisplayitem.h"

#include "Whatsapp/util/utilities.h"

#include "client.h"

ContactDisplayItem::ContactDisplayItem(Contact *c) :
    QStandardItem()
{
    this->contact = c;

    QString name = (contact->fromAddressBook || contact->alias.isEmpty())
            ? (contact->name.isEmpty() ? contact->phone : contact->name)
            : contact->alias;

    QString status = c->status.replace("<","&lt;").replace(">","&gt;");

    QString html = "<table width=\"100%\"><tr><td>" +
                   Utilities::WATextToHtml(name,32) + "</td>"
                   "<td align=\"right\">";

    QColor color = QMaemo5Style::standardColor("SecondaryTextColor");

    if (Client::showNumbers)
        html.append("<div style=\"font-size:16px;color:" + color.name() + "\">(" +
                    c->phone + ")&nbsp;</div>");

    QTextDocument line;
    line.setHtml("<div style=\"font-size:18px;color:" + color.name() + "\">O</div>");

    html.append("</td></tr>"
                "<tr><td colspan=\"2\"><div style=\"font-size:18px;color:" + color.name() + "\">" +
                Utilities::WATextToHtml(status,line.documentLayout()->documentSize().height() - 8)
                + "</div></td></tr></table>");

    setEditable(false);

    setData(html,Qt::DisplayRole);
    setData(c->jid,Qt::UserRole + 1);

    if (c->photoId.isEmpty())
        setData(QImage("/usr/share/icons/hicolor/64x64/hildon/general_default_avatar.png"), Qt::UserRole);
    else
        setData(c->photo, Qt::UserRole);

}

Contact* ContactDisplayItem::getContact()
{
    return contact;
}
