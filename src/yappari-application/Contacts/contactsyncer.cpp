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

#include <QSslConfiguration>
#include <QRegExp>
#include <QImage>
#include <QUrl>

#include "Dbus/gtkbindings-new.h"
#include <libosso.h>
#include <libebook/e-book.h>
#include <libosso-abook/osso-abook.h>
#include <gdk/gdkpixbuf.h>

#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/qtmd5digest.h"

#include "qt-json/json.h"

#include "client.h"
#include "version.h"
#include "globalconstants.h"
#include "contactsyncer.h"
#include "contactlistiterator.h"

ContactSyncer::ContactSyncer(ContactRoster* roster,
                             QObject *parent)
    : HttpRequest(parent)
{
    isSyncing = false;

    this->roster = roster;

    osso_context_t *osso_context = osso_initialize(YAPPARI_APPLICATION_NAME,
                                                   VERSION,
                                                   FALSE, NULL);
    int argc = 0;
    char **argv;
    osso_abook_init(&argc,&argv,osso_context);
}

QString ContactSyncer::getAuthResponse(QString nonce)
{
    // New contact synchronization method

    QtMD5Digest digest;

    qint64 clock = QDateTime::currentMSecsSinceEpoch();

    QString cnonce = QString::number(clock,36);
    QString nc = "00000001";
    QString digestUri = "WAWA/" JID_DOMAIN;

    // Login information
    QByteArray password = (Client::password.isEmpty())
                                ? Utilities::getChatPassword().toUtf8()
                                : QByteArray::fromBase64(Client::password.toUtf8());

    QString loginStr = Client::phoneNumber + ":" + JID_DOMAIN + ":";
    QByteArray bArray = loginStr.toUtf8();
    bArray.append(password);
    digest.update(bArray);
    QByteArray loginArray = digest.digest();
    digest.reset();

    // Nonce and Cnonce information
    QString nonceStr = ":" + nonce + ":" + cnonce;
    loginArray.append(nonceStr.toUtf8());
    digest.update(loginArray);
    QByteArray authentication = encode(digest.digest());
    digest.reset();

    QString nonceauthStr = ":" + nonce + ":" + nc + ":" + cnonce + ":auth:";
    authentication.append(nonceauthStr.toUtf8());

    // Authentication information
    QString authStr = "AUTHENTICATE:" + digestUri;
    digest.update(authStr.toUtf8());
    authentication.append(encode(digest.digest()));
    digest.reset();

    digest.update(authentication);

    QString response = "X-WAWA: username=\"";

    response.append(Client::phoneNumber);
    response.append("\",realm=\"");
    response.append(JID_DOMAIN);
    response.append("\",nonce=\"");
    response.append(nonce);
    response.append("\",cnonce=\"");
    response.append(cnonce);
    response.append("\",nc=\"");
    response.append(nc);
    response.append("\",qop=\"auth\",digest-uri=\"");
    response.append(digestUri);
    response.append("\",response=\"");
    response.append(QUrl::toPercentEncoding(QString::fromUtf8(encode(digest.digest()))));
    //response.append(QString::fromUtf8(encode(digest.digest())));
    response.append("\",charset=\"utf-8\"");

    return response;
}

QByteArray ContactSyncer::encode(QByteArray bytes)
{
    QByteArray result;

    for (int j = 0; j < bytes.length(); j++)
    {
        int k = bytes.at(j);
        if (k < 0)
            k += 256;

        result.append(encodeByte(k >> 4));
        result.append(encodeByte(k % 16));
    }

    return result;
}

int ContactSyncer::encodeByte(int c)
{
    return (c < 10) ? c + 48 : c + 87;
}

void ContactSyncer::freeAddressBook()
{
    Utilities::logData("syncer: freeing address book");
    QStringList list = abook.keys();

    foreach (QString key, list)
    {
        Contact *c = abook.value(key);
        delete c;
    }

    delete listIterator;
    abook.clear();
}

void ContactSyncer::getAddressBook()
{
    GError *error = NULL;

    OssoABookRoster *o_roster = osso_abook_aggregator_get_default (&error);
    if (error)
    {
        QString errorStr = error->message;
        Utilities::logData("getAddressBook(): " + errorStr);
        g_error_free(error);
    }

    OssoABookAggregator *aggregator = OSSO_ABOOK_AGGREGATOR(o_roster);
    osso_abook_waitable_run(OSSO_ABOOK_WAITABLE(aggregator),
                            g_main_context_default(), &error);
    if (error)
    {
        QString errorStr = error->message;
        Utilities::logData("getAddressBook(): " + errorStr);
        g_error_free(error);
    }

    GList *contacts = osso_abook_aggregator_list_master_contacts(aggregator);


    for (GList *l = contacts;l ; l = l->next)
    {
        OssoABookContact *contact = OSSO_ABOOK_CONTACT(l->data);
        QImage image;

        QString displayName = QString::fromUtf8(osso_abook_contact_get_display_name(contact));

        GdkPixbuf *pixbuf = osso_abook_contact_get_photo(contact);

        if (pixbuf)
        {
            gchar *buffer;
            gsize buffer_size;
            GError *error = 0;
            gdk_pixbuf_save_to_bufferv(pixbuf,&buffer,&buffer_size,"png",NULL,NULL,&error);

            QByteArray data = QByteArray::fromRawData(buffer,buffer_size);
            image.loadFromData(data);
            g_free(buffer);

            gdk_pixbuf_unref(pixbuf);
        }

        EContactField field = e_contact_field_id("mobile-phone");
        GList *attrs = e_contact_get_attributes(E_CONTACT(contact),field);
        for (GList *la = attrs; la; la = la->next)
        {
            EVCardAttribute *attr = (EVCardAttribute *) la->data;

            gchar *str = e_vcard_attribute_get_value(attr);
            QString phoneNumber = QString::fromUtf8(str);
            g_free(str);

            // ToDo: Delete everything that's not a number

            if (Client::sync.isEmpty() || Client::sync == SYNC_ENABLED ||
                    (Client::sync == SYNC_INTL_ONLY && phoneNumber.left(1) == "+"))
            {
                Contact *c = new Contact();
                c->name = displayName;
                c->phone = phoneNumber;
                c->type = Contact::TypeContact;
                c->photo = image;
                c->fromAddressBook = true;
                abook.insert(c->phone,c);
            }
        }
        g_list_foreach(attrs, (GFunc) e_vcard_attribute_free, NULL);
        g_list_free(attrs);

    }

    g_list_free(contacts);
}

void ContactSyncer::sync()
{
    if (!isSyncing && Client::connectionStatus == Client::LoggedIn)
    {
        Utilities::logData("syncer: Synchronizing contacts...");

        isSyncing = true;

        abookJids.clear();
        ContactList list = roster->getContactList();
        ContactListIterator abookIterator(list);
        for (int i = 0; abookIterator.hasNext(); i++)
        {
            abookIterator.next();

            Contact *c = abookIterator.value();

            if (c->type == Contact::TypeContact)
            {
                if (c->fromAddressBook)
                {
                    // Save the JID to check if it was
                    // removed from the phone address book later
                    abookJids.insert(c->jid,true);
                }
                else
                {
                    // If it's a temporal contact add it to the
                    // address book to synchronize it.
                    // We need to create a new one because this
                    // is gonna be freed later
                    Contact *cc = roster->cloneContact(c);
                    abook.insert(cc->phone,cc);
                }
            }
        }

        getAddressBook();
        listIterator = new ContactListIterator(abook);

        QString response = getAuthResponse("0");

        connect(&manager,SIGNAL(finished(QNetworkReply*)),
                this,SLOT(authResponse(QNetworkReply*)));

        addHeader("Authorization", response);

        Utilities::logData("syncer: Authenticating...");
        get(QUrl(URL_CONTACTS_AUTH),true);
    }
}

void ContactSyncer::authResponse(QNetworkReply *reply)
{
    Utilities::logData("syncer: authResponse()");
    QString result = QString::fromUtf8(reply->readAll().constData());
    disconnect(reply, 0, 0, 0);
    disconnect(&manager,SIGNAL(finished(QNetworkReply*)),
               this,SLOT(authResponse(QNetworkReply*)));
    // Utilities::logData("Reply: " + result);

    QString authData = reply->rawHeader("www-authenticate");
    QString nonce;

    bool ok;
    QVariantMap mapResult = QtJson::parse(result, ok).toMap();

    reply->deleteLater();

    QString message = mapResult.value("message").toString();

    if (message == "next token")
    {
        // Get nonce
        QRegExp noncereg("nonce=\"([^\"]+)\"");

        int pos = noncereg.indexIn(authData,0);

        if (pos != -1)
        {
            nonce = noncereg.cap(1);
        }

        Utilities::logData("syncer: Authentication successful");
        Utilities::logData("nonce: " + nonce);

        QString response = getAuthResponse(nonce);

        connect(&manager,SIGNAL(finished(QNetworkReply*)),
                this,SLOT(onResponse(QNetworkReply*)));

        addHeader("Authorization", response);

        Utilities::logData("syncer: Sending contacts info...");

        // Synchronize all contacts

        if (listIterator->hasNext())
        {
            writeBuffer.clear();
            addParam("ut","all");
            addParam("t", "c");

            int j;
            for (j = 0; listIterator->hasNext(); j++)
            {
                listIterator->next();

                Contact *c = listIterator->value();

                if (c->type == Contact::TypeContact)
                    addParam("u[]",c->phone);
            }

            get(QUrl(URL_CONTACTS_SYNC),true);
        }
    }
    else
    {
        Utilities::logData("syncer: Authentication failed.");

        // Free everything
        freeAddressBook();
        isSyncing = false;
        emit syncFinished();
    }
}


void ContactSyncer::onResponse(QNetworkReply *reply)
{
    QString jsonStr = QString::fromUtf8(reply->readAll().constData());
    disconnect(reply, 0, 0, 0);
    reply->deleteLater();
    // Utilities::logData("Reply: " + jsonStr);
    Utilities::logData("syncer: Response received");

    bool ok;
    QVariantMap mapResult = QtJson::parse(jsonStr, ok).toMap();

    if (mapResult.contains("c"))
    {
        phoneList = mapResult.value("c").toList();
        totalPhones = phoneList.length();
        nextSignal = totalPhones * 10 / 100;
        QTimer::singleShot(0,this,SLOT(syncNextPhone()));
    }
    else
    {
        Utilities::logData("syncer: Synchronization failed");

        // Free everything
        freeAddressBook();
        isSyncing = false;
        emit syncFinished();
    }
}

void ContactSyncer::syncNextPhone()
{
    if (!phoneList.isEmpty())
    {
        QVariant element = phoneList.takeFirst();
        QVariantMap e = element.toMap();
        int w = e.value("w").toInt();
        if (w)
        {
            QString phone = e.value("p").toString();
            QString jid = e.value("n").toString() + "@" + JID_DOMAIN;

            Contact *c = abook.value(phone);

            if (c)
            {
                if (roster->isContactInRoster(jid))
                {
                    Contact &d = roster->getContact(jid);
                    c->alias = d.alias;
                    c->typingStatus = d.typingStatus;
                    c->isOnline = d.isOnline;
                    c->lastSeen = d.lastSeen;
                }

                c->jid = jid;
                c->statusTimestamp = e.value("t").toLongLong();
                c->status = e.value("s").toString();
                roster->insertContact(c);
                abookJids.remove(c->jid);

                /*
                Utilities::logData("Synchronized contact:\n"
                                   "Name: " + c->name +
                                   "\nPhone: " + c->phone +
                                   "\njid: " + c->jid +
                                   "\nStatus: " + c->status);
                */

            }
        }

        if (--nextSignal == 0)
        {
            emit progress(((totalPhones-phoneList.length())*100) / totalPhones);
            nextSignal = totalPhones * 10 / 100;
        }

        QTimer::singleShot(50,this,SLOT(syncNextPhone()));
    }
    else
    {
        // Remove contacts that were deleted from the address book
        QList<QString> deleted = abookJids.keys();
        foreach (QString jid, deleted)
            roster->deleteContact(jid);

        Utilities::logData("syncer: Contacts synchronization successful");

        // Free everything
        freeAddressBook();
        isSyncing = false;
        emit syncFinished();
    }
}


void ContactSyncer::addParam(QString name, QString value)
{
    writeBuffer.append(QUrl::toPercentEncoding(name));
    writeBuffer.append('=');
    writeBuffer.append(QUrl::toPercentEncoding(value));
    writeBuffer.append('&');
}

bool ContactSyncer::isSynchronizing()
{
    return isSyncing;
}

void ContactSyncer::error(QNetworkReply::NetworkError error)
{
    Utilities::logData("syncer: Contact Syncer Error: " + QString::number(error));

    // Free everything
    /*
    freeAddressBook();
    isSyncing = false;
    emit syncFinished();
    */
}
