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

ContactSyncer::ContactSyncer(ContactRoster* roster,
                             QObject *parent)
    : HttpRequestv2(parent)
{
    isSyncing = false;

    this->roster = roster;

    connect(this,SIGNAL(headersReceived(qint64)),
            this,SLOT(increaseDownloadCounter(qint64)));

    connect(this,SIGNAL(requestSent(qint64)),
            this,SLOT(increaseUploadCounter(qint64)));

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

    abook.clear();
}

void ContactSyncer::getAddressBook()
{
    Utilities::logData("Retrieving address book...");
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();

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
        QString displayName = QString::fromUtf8(osso_abook_contact_get_display_name(contact));

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
                // c->photo = image;
                c->fromAddressBook = true;
                abook.insert(c->phone,c);
            }
        }
        g_list_foreach(attrs, (GFunc) e_vcard_attribute_free, NULL);
        g_list_free(attrs);

    }

    g_list_free(contacts);

    qint64 endTime = QDateTime::currentMSecsSinceEpoch() - startTime;
    Utilities::logData("Address book retrieved in " + QString::number(endTime) +
                       " milliseconds.");
}

void ContactSyncer::sync()
{
    if (!isSyncing && Client::connectionStatus == Client::LoggedIn)
    {
        Utilities::logData("syncer: Synchronizing contacts...");

        isSyncing = true;
        syncDataReceived = false;
        emit progress(0);

        deletedJids.clear();
        ContactList list = roster->getContactList();
        foreach (Contact *c, list.values())
        {
            if (c->type == Contact::TypeContact)
            {
                if (c->fromAddressBook)
                {
                    // Save the JID to check if it was
                    // removed from the phone address book later
                    deletedJids.insert(c->jid,true);
                }
                else
                {
                    // If it's a temporal contact include it to the
                    // address book to synchronize it.
                    // We need to create a new one because this
                    // is gonna be freed later
                    if (!abook.contains(c->jid))
                    {
                        Contact *cc = roster->cloneContact(c);
                        abook.insert(cc->phone,cc);
                    }
                }
            }
        }

        // Fill the abook with the phone's address book
        getAddressBook();

        // Sync the abook
        syncAddressBook();
    }
}

void ContactSyncer::syncContact(Contact *c)
{
    if (!isSyncing && Client::connectionStatus == Client::LoggedIn)
    {
        Utilities::logData("syncer: Synchronizing contact: " + c->jid);
        deletedJids.clear();
        isSyncing = true;
        syncDataReceived = false;

        // Only include a copy of the contact in the abook
        // It is gonna be freed later
        Contact *cc = roster->cloneContact(c);
        abook.insert(cc->phone,cc);

        syncAddressBook();
    }
}

void ContactSyncer::syncAddressBook()
{
    QString response = getAuthResponse("0");

    connect(this,SIGNAL(finished()),this,SLOT(authResponse()));
    connect(this,SIGNAL(socketError(QAbstractSocket::SocketError)),
            this,SLOT(errorHandler(QAbstractSocket::SocketError)));

    setHeader("Authorization", response);

    Utilities::logData("syncer: Authenticating...");
    post(QUrl(URL_CONTACTS_AUTH), writeBuffer.constData(), writeBuffer.length());
}

void ContactSyncer::authResponse()
{
    Utilities::logData("syncer: authResponse()");
    QString result = QString::fromUtf8(socket->readAll().constData());
    disconnect(this, SIGNAL(finished()), this, SLOT(authResponse()));

    // Utilities::logData("Reply: " + result);

    QString authData = getHeader("WWW-Authenticate");
    // Utilities::logData("authData: " + authData);
    QString nonce;

    clearHeaders();

    bool ok;
    QVariantMap mapResult = QtJson::parse(result, ok).toMap();

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

        connect(this,SIGNAL(finished()),this,SLOT(onResponse()));

        setHeader("Authorization", response);

        Utilities::logData("syncer: Sending contacts info...");

        // Synchronize all contacts

        if (abook.size() > 0)
        {
            writeBuffer.clear();
            addParam("ut","wa");
            addParam("t", "c");

            foreach (Contact *c, abook.values())
            {
                if (c->type == Contact::TypeContact)
                    addParam("u[]",c->phone);
            }

            post(QUrl(URL_CONTACTS_SYNC),writeBuffer.constData(), writeBuffer.length());
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

void ContactSyncer::onResponse()
{
    if (errorCode != 200)
    {
        emit httpError(errorCode);
        return;
    }

    totalLength = getHeader("Content-Length").toLongLong();

    Utilities::logData("syncer: Content-Length: " + QString::number(totalLength));

    if (socket->bytesAvailable())
        fillBuffer();

    connect(socket,SIGNAL(readyRead()),this,SLOT(fillBuffer()));
}

void ContactSyncer::fillBuffer()
{
    Utilities::logData("syncer: fillBuffer()");

    qint64 bytesToRead = socket->bytesAvailable();

    Utilities::logData("syncer: bytesAvailable(): " + QString::number(bytesToRead));

    readBuffer.append(socket->read(bytesToRead));

    Utilities::logData("syncer: readBuffer.size(): " + QString::number(readBuffer.size()));

    if (readBuffer.size() == totalLength)
    {
        Utilities::logData("syncer: Read " + QString::number(totalLength) + " bytes.");

        increaseDownloadCounter(totalLength);

        disconnect(this,SIGNAL(socketError(QAbstractSocket::SocketError)),
                   this,SLOT(errorHandler(QAbstractSocket::SocketError)));

        syncDataReceived = true;
        socket->close();

        parseResponse();
    }
}


void ContactSyncer::parseResponse()
{
    QString jsonStr = QString::fromUtf8(readBuffer.constData());
    Utilities::logData("syncer: Response received");

    // Utilities::logData("Reply: " + jsonStr);

    bool ok;
    QVariantMap mapResult = QtJson::parse(jsonStr, ok).toMap();

    if (mapResult.contains("c"))
    {
        phoneList = mapResult.value("c").toList();
        totalPhones = phoneList.length();
        nextSignal = totalPhones * 10 / 100;
        emit progress(5);
        QTimer::singleShot(100,this,SLOT(syncNextPhone()));
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
            // Original Number
            QString phone = e.value("p").toString();

            // Normalized Number
            QString jid = e.value("n").toString() + "@" + JID_DOMAIN;

            Contact *c = abook.value(phone);

            Contact *contact;

            if (c)
            {
                bool exists = roster->isContactInRoster(jid);
                bool updated = false;

                if (exists)
                {
                    Contact &d = roster->getContact(jid);
                    contact = &d;

                    // Check if the name was changed in the address book
                    if (contact->name != c->name)
                    {
                        updated = true;
                        contact->name = c->name;
                    }

                    // Check if this contact wasn't in the address book
                    // before but now it is
                    if (contact->fromAddressBook != c->fromAddressBook)
                    {
                        updated = true;
                        contact->fromAddressBook = c->fromAddressBook;
                    }
                }
                else
                {
                    contact = new Contact();
                    contact->fromAddressBook = true;
                    contact->name = c->name;
                    contact->phone = c->phone;
                    contact->jid = jid;
                    contact->photoId = QString();
                }

                // Status Timestamp
                qint64 statusTimestamp = e.value("t").toLongLong();
                if (contact->statusTimestamp != statusTimestamp)
                {
                    updated = true;
                    contact->statusTimestamp = statusTimestamp;

                    // Status
                    contact->status = e.value("s").toString();

                    emit statusChanged(contact->jid, contact->status);
                }

                if (exists && updated)
                    roster->updateContact(contact);
                else if (!exists)
                    roster->insertContact(contact);

                deletedJids.remove(jid);

                // if (contact->photoId.isEmpty() || contact->photoId == "abook")
                    emit photoRefresh(jid, contact->photoId, false);

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
        foreach (QString jid, deletedJids.keys())
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

void ContactSyncer::errorHandler(QAbstractSocket::SocketError error)
{
    // If all the synchronization data has been received ignore
    // the socket errors.  It looks like QSslSocket will still send
    // socket errors even if the socket has been closed.
    // Not sure if a bug or a feature but this way we avoid false positives
    if (!syncDataReceived)
    {
        if (error == QAbstractSocket::SslHandshakeFailedError)
        {
            // SSL error is a fatal error
            emit sslError();
        }
        else
        {
            // ToDo: Retry here
            Utilities::logData("syncer: Socket error while trying to get synchronization data.  Error: " + QString::number(error));
            emit httpError((int)error);
        }
    }
}

void ContactSyncer::increaseUploadCounter(qint64 bytes)
{
    Client::dataCounters.increaseCounter(DataCounters::SyncBytes, 0, bytes);
}

void ContactSyncer::increaseDownloadCounter(qint64 bytes)
{
    Client::dataCounters.increaseCounter(DataCounters::SyncBytes, bytes, 0);
}
