/**
 * Copyright (C) 2013 Naikel Aparicio. All rights reserved.
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

#include <QRegExp>
#include <QTime>

#include "Whatsapp/util/qtmd5digest.h"
#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/protocoltreenodelistiterator.h"

#include "globalconstants.h"

#include "connection.h"

#include "client.h"

FunStore Connection::store;

/**
    Constructs a Connection object.

    @param socket       A QTcpSocket with a established TCP connection.
    @param domain       Usually "s.whatsapp.net"
    @param resource     String that describes the client used.
    @param user         Phone number.
    @param push_name    User name or alias.
    @param password     Password.
    @param counters     Pointer to a DataCounters class whre counters are kept.
    @param parent       QObject parent.
*/
Connection::Connection(QTcpSocket *socket, QString domain, QString resource,
                       QString user, QString push_name, QByteArray password,
                       DataCounters *counters, QObject *parent)
    : QObject(parent)
{

    /*
     * This is the dictionary Whatsapp uses to compress its data
     * so the packets are really tiny
     */
    dictionary << NULL << NULL << NULL << "account" << "ack" << "action" << "active" << "add" << "after" << "all" << "allow" << "apple" << "auth" << "author" << "available" << "bad-protocol" << "bad-request" << "before" << "body" << "broadcast" << "cancel" << "category" << "challenge" << "chat" << "clean" << "code" << "composing" << "config" << "contacts" << "count" << "create" << "creation" << "debug" << "default" << "delete" << "delivery" << "delta" << "deny" << "digest" << "dirty" << "duplicate" << "elapsed" << "enable" << "encoding" << "error" << "event" << "expiration" << "expired" << "fail" << "failure" << "false" << "favorites" << "feature" << "features" << "feature-not-implemented" << "field" << "first" << "free" << "from" << "g.us" << "get" << "google" << "group" << "groups" << "http://etherx.jabber.org/streams" << "http://jabber.org/protocol/chatstates" << "ib" << "id" << "image" << "img" << "index" << "internal-server-error" << "ip" << "iq" << "item-not-found" << "item" << "jabber:iq:last" << "jabber:iq:privacy" << "jabber:x:event" << "jid" << "kind" << "last" << "leave" << "list" << "max" << "mechanism" << "media" << "message_acks" << "message" << "method" << "microsoft" << "missing" << "modify" << "mute" << "name" << "nokia" << "none" << "not-acceptable" << "not-allowed" << "not-authorized" << "notification" << "notify" << "off" << "offline" << "order" << "owner" << "owning" << "p_o" << "p_t" << "paid" << "participant" << "participants" << "participating" << "paused" << "picture" << "pin" << "ping" << "platform" << "port" << "presence" << "preview" << "probe" << "prop" << "props" << "query" << "raw" << "read" << "reason" << "receipt" << "received" << "relay" << "remote-server-timeout" << "remove" << "request" << "required" << "resource-constraint" << "resource" << "response" << "result" << "retry" << "rim" << "s_o" << "s_t" << "s.us" << "s.whatsapp.net" << "seconds" << "server-error" << "server" << "service-unavailable" << "set" << "show" << "silent" << "stat" << "status" << "stream:error" << "stream:features" << "subject" << "subscribe" << "success" << "sync" << "t" << "text" << "timeout" << "timestamp" << "to" << "true" << "type" << "unavailable" << "unsubscribe" << "uri" << "url" << "urn:ietf:params:xml:ns:xmpp-sasl" << "urn:ietf:params:xml:ns:xmpp-stanzas" << "urn:ietf:params:xml:ns:xmpp-streams" << "urn:xmpp:ping" << "urn:xmpp:receipts" << "urn:xmpp:whatsapp:account" << "urn:xmpp:whatsapp:dirty" << "urn:xmpp:whatsapp:mms" << "urn:xmpp:whatsapp:push" << "urn:xmpp:whatsapp" << "user" << "user-not-found" << "value" << "version" << "w:g" << "w:p:r" << "w:p" << "w:profile:picture" << "w" << "wait" << "WAUTH-2" << "x" << "xmlns:stream" << "xmlns" << "1" << "chatstate" << "crypto" << "enc" << "class" << "off_cnt" << "w:g2" << "promote" << "demote" << "creator" << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << NULL << "Bell.caf" << "Boing.caf" << "Glass.caf" << "Harp.caf" << "TimePassing.caf" << "Tri-tone.caf" << "Xylophone.caf" << "background" << "backoff" << "chunked" << "context" << "full" << "in" << "interactive" << "out" << "registration" <<  "sid" << "urn:xmpp:whatsapp:sync" << "flt" << "s16" << "u8" << "adpcm" << "amrnb" << "amrwb" << "mp3" << "pcm" << "qcelp" << "wma" << "h263" << "h264" << "jpeg" << "mpeg4" <<  "wmv" << "audio/3gpp" << "audio/aac" << "audio/amr" << "audio/mp4" << "audio/mpeg" << "audio/ogg" << "audio/qcelp" << "audio/wav" << "audio/webm" << "audio/x-caf" << "audio/x-ms-wma" << "image/gif" << "image/jpeg" << "image/png" << "video/3gpp" <<  "video/avi" << "video/mp4" << "video/mpeg" << "video/quicktime" << "video/x-flv" << "video/x-ms-asf" << "302" << "400" << "401" << "402" << "403" << "404" << "405" << "406" << "407" << "409" <<  "500" << "501" << "503" << "504" << "abitrate" << "acodec" << "app_uptime" << "asampfmt" << "asampfreq" << "audio" << "bb_db" << "clear" << "conflict" << "conn_no_nna" << "cost" << "currency" <<  "duration" << "extend" << "file" << "fps" << "g_notify" << "g_sound" << "gcm" << "google_play" << "hash" << "height" << "invalid" << "jid-malformed" << "latitude" << "lc" << "lg" << "live" <<  "location" << "log" << "longitude" << "max_groups" << "max_participants" << "max_subject" << "mimetype" << "mode" << "napi_version" << "normalize" << "orighash" << "origin" << "passive" << "password" << "played" << "policy-violation" <<  "pop_mean_time" << "pop_plus_minus" << "price" << "pricing" << "redeem" << "Replaced by new connection" << "resume" << "signature" << "size" << "sound" << "source" << "system-shutdown" << "username" << "vbitrate" << "vcard" << "vcodec" <<  "video" << "width" << "xml-not-well-formed" << "checkmarks" << "image_max_edge" << "image_max_kbytes" << "image_quality" << "ka" << "ka_grow" << "ka_shrink" << "newmedia" << "library" << "caption" << "forward" << "c0" << "c1" <<  "c2" << "c3" << "clock_skew" << "cts" << "k0" << "k1" << "login_rtt" << "m_id" << "nna_msg_rtt" << "nna_no_off_count" << "nna_offline_ratio" << "nna_push_rtt" << "no_nna_con_count" << "off_msg_rtt" << "on_msg_rtt" << "stat_name" <<  "sts" << "suspect_conn" << "lists" << "self" << "qr" << "web" << "w:b" << "recipient" << "w:stats" << "forbidden" << "aurora.m4r" << "bamboo.m4r" << "chord.m4r" << "circles.m4r" << "complete.m4r" << "hello.m4r" <<  "input.m4r" << "keys.m4r" << "note.m4r" << "popcorn.m4r" << "pulse.m4r" << "synth.m4r" << "filehash";

    this->socket = socket;
    this->user = user;
    this->domain = domain;
    this->resource = resource;
    this->push_name = push_name;
    this->password = password;
    this->iqid = 0;
    this->counters = counters;
    this->myJid = user + "@" + JID_DOMAIN;

    this->out = new BinTreeNodeWriter(socket,dictionary,this);
    this->in = new BinTreeNodeReader(socket,dictionary,this);

    connect(&connTimeout,SIGNAL(timeout()),this,SLOT(connectionTimeout()));
    connTimeout.start(CHECK_CONNECTION_INTERVAL);
}

/**
    Destroys a Connection object
*/
Connection::~Connection()
{
    /**
        @todo Clean disconnect to the WhatsApp servers
     */
}

/**
    Login to the WhatsApp service.

    @param nextChallenge    Next authentication challente to use.
*/
void Connection::login(QByteArray nextChallenge)
{
    // Update the challenge
    this->nextChallenge = nextChallenge;

    int outBytes, inBytes;
    outBytes = inBytes = 0;

    outBytes = out->streamStart(domain,resource);
    outBytes += sendFeatures();
    outBytes += sendAuth();
    inBytes += in->readStreamStart();
    QByteArray challengeData = readFeaturesUntilChallengeOrSuccess(&inBytes);
    if (challengeData.size() > 0)
    {
        outBytes += sendResponse(challengeData);
        inBytes += readSuccess();
    }

    counters->increaseCounter(DataCounters::ProtocolBytes, inBytes, outBytes);

    // Successful login at this point

    sendClientConfig("none");
    sendAvailableForChat();
}

/**
    Reads the next node.

    @return     true if a new node was read successfully.
*/
bool Connection::read()
{
    ChatMessageType msgType = Unknown;

    ProtocolTreeNode node;
    bool pictureReceived = false;
    bool synchronization = false;

    if (in->nextTree(node))
    {
        lastTreeRead = QDateTime::currentMSecsSinceEpoch();
        connTimeout.start(CHECK_CONNECTION_INTERVAL);
        QString tag = node.getTag();

        if (tag == "iq")
        {
            QString type = node.getAttributeValue("type");
            QString id = node.getAttributeValue("id");
            QString from = node.getAttributeValue("from");

            if (type == "get")
            {
                ProtocolTreeNodeListIterator i(node.getChildren());
                if (i.hasNext())
                {
                    ProtocolTreeNode child = i.next().value();
                    if (child.getTag()=="ping")
                        sendPong(id);
                }
            }
            else if (type == "result")
            {
                QStringList privacyList;
                bool isPrivacyList = false;

                ProtocolTreeNodeListIterator i(node.getChildren());
                while (i.hasNext())
                {
                    ProtocolTreeNode child = i.next().value();

                    if (child.getTag() == "group")
                    {
                        QString childId = child.getAttributeValue("id");
                        QString subject = child.getAttributeValue("subject");
                        QString author = child.getAttributeValue("owner");
                        QString creation = child.getAttributeValue("creation");
                        QString subject_o = child.getAttributeValue("s_o");
                        QString subject_t = child.getAttributeValue("s_t");
                        QString jid = childId + "@g.us";

                        QStringList groupParticipants;
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode participant = j.next().value();
                            if (participant.getTag() == "participant")
                            {
                                QString jid = participant.getAttributeValue("jid");
                                if (participant.getAttributeValue("type")=="admin")
                                    groupParticipants.append(jid+"(groupadmin)");
                                else
                                    groupParticipants.append(jid);
                            }
                        }
                        // need to add participants to signal!
                        //emit groupInfoFromList(jid, author, subject, creation, subject_o, subject_t, groupParticipants);
                        emit groupInfoFromList(id, jid, author, subject, creation, subject_o, subject_t);
                    }

                    if (child.getTag() == "list")
                    {
                        QString childId = child.getAttributeValue("id");
                        QString subject = child.getAttributeValue("name");

                        QStringList groupParticipants;
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode participant = j.next().value();
                            if (participant.getTag() == "recipient")
                            {
                                QString jid = participant.getAttributeValue("jid");
                                groupParticipants.append(jid);
                            }
                        }
                        //broadcast lists - not implemented yet in yappari
                        //emit broadcastInfoFromList(childId, subject, groupParticipants);
                    }

                    if (child.getTag() == "delete" )
                    {
                        if (id.contains("delete_broadcast_"))
                        {
                            //broadcast lists - not implemented yet in yappari
                            //emit deleteList(toDelete);
                        }
                    }

                    if (child.getTag() == "leave")
                    {
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode group = j.next().value();
                            if (group.getTag() == "group")
                            {
                                QString groupId = group.getAttributeValue("id");
                                emit groupLeft(groupId);
                            }
                        }
                    }

                    else if (child.getTag() == "query")
                    {
                        if (child.getAttributeValue("seconds").toLongLong() > 0) {
                            qint64 timestamp = QDateTime::currentMSecsSinceEpoch() -
-                                               (child.getAttributeValue("seconds").toLongLong() * 1000);
                            emit lastOnline(from, timestamp);
                        }

                    }


                    else if (child.getTag() == "media" || child.getTag() == "duplicate")
                    {
                        Key k(JID_DOMAIN,true,id);
                        FMessage message = store.value(k);

                        if (message.key.id == id)
                        {
                            message.status = (child.getTag() == "media")
                                        ? FMessage::Uploading
                                        : FMessage::Uploaded;
                            message.media_url = child.getAttributeValue("url");
                            message.media_mime_type = child.getAttributeValue("mimetype");
                            if (message.media_wa_type == FMessage::Video ||
                                message.media_wa_type == FMessage::Audio)
                            {
                                QString duration = child.getAttributeValue("duration");
                                message.media_duration_seconds =
                                        (duration.isEmpty()) ? 0 : duration.toInt();
                            }

                            store.remove(k);

                            emit mediaUploadAccepted(message);

                        }
                    }

                    // This is the result of the sendGetPhotoIds()
                    // That method is not used anymore

                    else if (child.getTag() == "list")
                    {
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode user = j.next().value();
                            if (user.getTag() == "user")
                            {
                                QString jid = user.getAttributeValue("jid");
                                QString pictureId = user.getAttributeValue("id");

                                emit photoIdReceived(jid, QString(), pictureId);
                            }
                        }
                    }

                    else if (child.getTag() == "picture")
                    {
                        QString imageType = child.getAttributeValue("type");
                        QString photoId = child.getAttributeValue("id");
                        QByteArray bytes = child.getData();

                        emit photoReceived(from, bytes,
                                           photoId, (imageType == "image"));

                        pictureReceived = true;
                        counters->increaseCounter(DataCounters::ProfileBytes, node.getSize(), 0);
                    }

                    else if (child.getTag() == "add")
                    {
                        QString participant = child.getAttributeValue("participant");

                        if (child.getAttributeValue("type") == "success")
                            emit groupAddUser(from, participant);
                    }

                    else if (child.getTag() == "participant")
                    {
                        QString participant = child.getAttributeValue("jid");

                        emit groupUser(from, participant);
                    }

                    else if (child.getTag() == "sync")
                    {
                        synchronization = true;
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode syncIn = j.next().value();
                            if (syncIn.getTag() == "in")
                            {
                                ProtocolTreeNodeListIterator k(syncIn.getChildren());
                                while (k.hasNext())
                                {
                                    ProtocolTreeNode user = k.next().value();
                                    if (user.getTag() == "user")
                                    {
                                        QString jid = user.getAttributeValue("jid");
                                        QString phone = user.getDataString();

                                        emit contactSync(jid, phone);
                                    }

                                }
                            }
                            else if (syncIn.getTag() == "out")
                            {
                                ProtocolTreeNodeListIterator k(syncIn.getChildren());
                                while (k.hasNext())
                                {
                                    ProtocolTreeNode user = k.next().value();
                                    if (user.getTag() == "user")
                                    {
                                        QString jid = user.getAttributeValue("jid");
                                        QString phone = user.getDataString();

                                        Utilities::logData("Contact deleted: " + jid + " "  + phone);

                                        emit contactDeleted(jid, phone);
                                    }
                                }
                            }
                            else if (syncIn.getTag() == "invalid")
                            {
                                ProtocolTreeNodeListIterator k(syncIn.getChildren());
                                while (k.hasNext())
                                {
                                    ProtocolTreeNode user = k.next().value();
                                    if (user.getTag() == "user")
                                    {
                                        QString jid = user.getAttributeValue("jid");
                                        QString phone = user.getDataString();

                                        Utilities::logData("Contact invalid: " + jid + " "  + phone);

                                        emit contactDeleted(jid, phone);
                                    }
                                }
                            }
                        }

                        counters->increaseCounter(DataCounters::SyncBytes, node.getSize(), 0);
                    }

                    else if (child.getTag() == "status")
                    {
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode user = j.next().value();
                            if (user.getTag() == "user")
                            {
                                QString jid = user.getAttributeValue("jid");
                                qint64 t = user.getAttributeValue("t").toLongLong();
                                QString status = user.getDataString();

                                emit statusChanged(jid, t, status);
                            }
                        }
                    }
                }

                if (id.left(10) == "set_photo_" && from == myJid &&
                    node.getChildrenCount() == 0)
                {
                    emit photoDeleted(myJid, QString());
                }

                if (isPrivacyList)
                    emit privacyListReceived(privacyList);
            }
            else if (type == "error")
            {
               QString id = node.getAttributeValue("id");
               if (id.left(10) == "get_photo_")
                   emit photoDeleted(from, QString());
               else if (id.left(11) == "privacylist")
                   emit privacyListReceived(QStringList());
               else if (id.left(5) == "sync_")
                   emit syncError();

            }
        }

        if (tag == "presence")
        {
            QString xmlns = node.getAttributeValue("xmlns");
            QString from = node.getAttributeValue("from");
            QString last = node.getAttributeValue("last");
            if ((xmlns.isEmpty() || xmlns == "urn:xmpp") && !from.isEmpty() && from != myJid)
            {
                QString type = node.getAttributeValue("type");
                if (type.isEmpty() || type == "available")
                    emit available(from, true);
                else if (type == "unavailable")
                    emit available(from, false);

                if (type=="unavailable" ) {
                    if (last!="deny" && !last.isEmpty() && last!="") {
                        emit lastOnline(from, last.toLongLong()*1000);
                    }
                }
            }
            else if (xmlns == "w" && !from.isEmpty())
            {
                QString add = node.getAttributeValue("add");
                QString remove = node.getAttributeValue("remove");
                if (!add.isEmpty())
                    emit groupAddUser(from, add);
                else if (!remove.isEmpty())
                    emit groupRemoveUser(from, remove);
            }
        }


        if (tag == "chatstate") {
            QString from = node.getAttributeValue("from");
            QString participant = node.getAttributeValue("participant");
            ProtocolTreeNodeListIterator i(node.getChildren());
            while (i.hasNext()) {
                ProtocolTreeNode child = i.next().value();
                if (child.getTag() == "composing") {
                    emit composing(from, participant, child.getAttributeValue("media"));
                }
                else if (child.getTag() == "paused") {
                    emit paused(from, participant);
                }
            }
        }


        if (tag == "ack") {
            QString aclass = node.getAttributeValue("class");
            if (aclass == "message")
            {
                //Sent message received by server

                QString from = node.getAttributeValue("from");
                QString id = node.getAttributeValue("id");
                QString count = node.getAttributeValue("count");

                FMessage message;
                Key k(from, true, id);
                message = store.value(k);
                message.status = FMessage::ReceivedByServer;
                message.count = count.toInt();

                msgType = MessageStatusUpdate;

                // If this a receipt from a group message
                // delete it from the FunStore
                if (from.right(5) == "@g.us")
                    store.remove(k);

                emit messageStatusUpdate(message);

            }
            else if (aclass ==  "receipt")
            {
                //Received message notification for received/played/read by me
                //Not implemented yet

                /*QString from = node.getAttributeValue("from");
                QString id = node.getAttributeValue("id");
                QString type = node.getAttributeValue("type");
                QString participant = node.getAttributeValue("participant");
                FMessage message;
                Key k(from, false, id);
                message.key = k;
                message.remote_resource = participant;
                if (type.isEmpty()) message.status = FMessage::ReceivedByTarget;
                if (type=="played") message.status = FMessage::Played;
                if (type=="read" && Client::blueChecks) message.status = FMessage::ReadByTarget;

                msgType = (from == "s.us") ? Unknown : MessageStatusUpdate;

                // Remove it from the store if it's not a voice message
                // Or if it's a voice message already played
                if ((message.live && receipt_type == "played") || !message.live)
                    store.remove(k);

                // But restore it if still waiting for ReadByTarget
                if (message.status == FMessage::ReceivedByTarget)
                    store.put(message);*/
            }
        }

        if (tag == "receipt")
        {
            //Sent message received/played/read by target

            QString from = node.getAttributeValue("from");
            QString id = node.getAttributeValue("id");
            QString receipt_type = node.getAttributeValue("type");
            QString participant = node.getAttributeValue("participant");
            QString attribute_t = node.getAttributeValue("t");

            FMessage message;
            Key k(from,true,id);
            message = store.value(k);
            if (message.key.id == id)
            {
                message.status = (receipt_type == "played")
                        ? FMessage::Played
                        /* Whatsapp server sends the same "received" message twice:
                           once when received by the target and the second one when
                           it has been read (or at least displayed by the app) */
                        : (message.status == FMessage::ReceivedByTarget && Client::blueChecks)
                          ? FMessage::ReadByTarget
                          : FMessage::ReceivedByTarget;
                msgType = (from == "s.us") ? Unknown : MessageStatusUpdate;

                // Remove it from the store if it's not a voice message
                // Or if it's a voice message already played
                if ((message.live && receipt_type == "played") || !message.live)
                    store.remove(k);

                // But restore it if still waiting for ReadByTarget
                if (message.status == FMessage::ReceivedByTarget)
                    store.put(message);
            }
            if (receipt_type == "delivered" || receipt_type == "played" ||
                receipt_type.isEmpty())
            {
                // Delivery Receipt received
                sendDeliveredReceiptAck(from,id,
                                        (receipt_type.isEmpty()
                                         ? "delivered"
                                         : receipt_type));
            }

            emit messageStatusUpdate(message);

            //READ LIST
            if (receipt_type=="read" || receipt_type=="played")
            {
                ProtocolTreeNodeListIterator i(node.getChildren());
                while (i.hasNext())
                {
                    ProtocolTreeNode child = i.next().value();
                    if (child.getTag() == "list")
                    {
                        ProtocolTreeNodeListIterator j(child.getChildren());
                        while (j.hasNext())
                        {
                            ProtocolTreeNode user = j.next().value();
                            if (user.getTag()=="item") {
                                QString kid = user.getAttributeValue("id");

                                FMessage message;
                                Key k(from,true,kid);
                                message = store.value(k);
                                if (message.key.id == kid)
                                {
                                    message.status = (receipt_type == "played")
                                            ? FMessage::Played
                                            /* Whatsapp server sends the same "received" message twice:
                                               once when received by the target and the second one when
                                               it has been read (or at least displayed by the app) */
                                            : (message.status == FMessage::ReceivedByTarget && Client::blueChecks)
                                              ? FMessage::ReadByTarget
                                              : FMessage::ReceivedByTarget;
                                    msgType = (from == "s.us") ? Unknown : MessageStatusUpdate;

                                    // Remove it from the store if it's not a voice message
                                    // Or if it's a voice message already played
                                    if ((message.live && receipt_type == "played") || !message.live)
                                        store.remove(k);

                                    // But restore it if still waiting for ReadByTarget
                                    if (message.status == FMessage::ReceivedByTarget)
                                        store.put(message);
                                }
                                if (receipt_type == "delivered" || receipt_type == "played" ||
                                    receipt_type.isEmpty())
                                {
                                    // Delivery Receipt received
                                    sendDeliveredReceiptAck(from, kid,
                                                            (receipt_type.isEmpty()
                                                             ? "delivered"
                                                             : receipt_type));
                                }

                                emit messageStatusUpdate(message);
                            }
                        }
                    }

                }

            }
            // Delivery Receipt received
            sendDeliveredReceiptAck(from, id, receipt_type, participant);

        }
        
        if (tag == "notification")
        {
            QString type = node.getAttributeValue("type");
            QString from = node.getAttributeValue("from");
            QString to = node.getAttributeValue("to");
            QString participant = node.getAttributeValue("participant");
            QString id = node.getAttributeValue("id");
            QString notify = node.getAttributeValue("notify");

            if (type == "picture")
            {
                ProtocolTreeNodeListIterator k(node.getChildren());
                while (k.hasNext())
                {
                    ProtocolTreeNode l = k.next().value();

                    if (l.getTag() == "set")
                    {
                        QString photoId = l.getAttributeValue("id");
                        if (!photoId.isEmpty())
                        {
                            sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                            emit photoIdReceived(from, notify, photoId);
                        }
                    }
                    else if (l.getTag() == "delete")
                    {
                        sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                        emit photoDeleted(from, notify);
                    }
                }
            }

            if (type == "status")
            {
                ProtocolTreeNodeListIterator j(node.getChildren());
                while (j.hasNext())
                {
                    ProtocolTreeNode user = j.next().value();
                    if (user.getTag()=="set") {
                        sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                        emit statusChanged(from, node.getAttributeValue("t").toInt(), user.getData());
                    }
                }
            }

            if (type == "contacts") {
                // Not implemented
                sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
            }


            if (type == "w:gp2")
            {
                ProtocolTreeNodeListIterator k(node.getChildren());
                while (k.hasNext())
                {
                    ProtocolTreeNode l = k.next().value();
                    if (l.getTag() == "create")
                    {
                        ProtocolTreeNodeListIterator m(l.getChildren());
                        while (m.hasNext())
                        {
                            ProtocolTreeNode n = m.next().value();
                            if (n.getTag() == "group")
                            {
                                QString childId = n.getAttributeValue("id");
                                QString subject = n.getAttributeValue("subject");
                                QString author = n.getAttributeValue("creator");
                                QString creation = n.getAttributeValue("creation");
                                QString subject_o = n.getAttributeValue("s_o");
                                QString subject_t = n.getAttributeValue("s_t");
                                QString jid = childId + "@g.us";

                                QStringList groupParticipants;
                                ProtocolTreeNodeListIterator o(n.getChildren());
                                while (o.hasNext())
                                {
                                    ProtocolTreeNode participant = o.next().value();
                                    if (participant.getTag() == "participant")
                                    {
                                        QString jid = participant.getAttributeValue("jid");
                                        groupParticipants.append(jid);
                                    }
                                }
                                Utilities::logData("Created new group: " + subject  + " - Participants: " + groupParticipants.join(","));
                                sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                                emit groupInfoFromList(id, jid, author, subject, creation, subject_o, subject_t);
                            }
                        }
                    }
                    else if (l.getTag() == "add")
                    {
                        ProtocolTreeNodeListIterator m(l.getChildren());
                        while (m.hasNext())
                        {
                            ProtocolTreeNode participantChild = m.next().value();
                            if (participantChild.getTag() == "participant") {
                                QString jid = participantChild.getAttributeValue("jid");
                                if (jid == myJid) {
                                    //sendGetGroupInfo(from);
                                }
                                else if (!jid.isEmpty()) {
                                    sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                                    emit groupAddUser(from, jid);
                                }
                            }
                        }
                    }
                    else if (l.getTag() == "remove")
                    {
                        ProtocolTreeNodeListIterator m(l.getChildren());
                        while (m.hasNext())
                        {
                            ProtocolTreeNode participantChild = m.next().value();
                            if (participantChild.getTag() == "participant") {
                                QString jid = participantChild.getAttributeValue("jid");
                                if (!jid.isEmpty()) {
                                    sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                                    if (jid!=myJid)
                                        emit groupRemoveUser(from, jid);
                                }
                            }
                        }
                    }
                    else if (l.getTag() == "promote")
                    {
                        ProtocolTreeNodeListIterator m(l.getChildren());
                        while (m.hasNext())
                        {
                            ProtocolTreeNode participantChild = m.next().value();
                            if (participantChild.getTag() == "participant") {
                                QString jid = participantChild.getAttributeValue("jid");
                                if (!jid.isEmpty()) {
                                    sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                                    // Not implemented yet
                                    //emit groupAddAdminUser(from, jid);
                                }
                            }
                        }
                    }
                    else if (l.getTag() == "demote")
                    {
                        ProtocolTreeNodeListIterator m(l.getChildren());
                        while (m.hasNext())
                        {
                            ProtocolTreeNode participantChild = m.next().value();
                            if (participantChild.getTag() == "participant") {
                                QString jid = participantChild.getAttributeValue("jid");
                                if (!jid.isEmpty()) {
                                    sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                                    // Not implemented yet
                                    //emit groupRemoveAdminUser(from, jid);
                                }
                            }
                        }
                    }

                    else if (l.getTag() == "subject")
                    {
                        //QString subject_o = l.getAttributeValue("s_o");
                        QString subject_t = l.getAttributeValue("s_t");
                        QString subject = l.getAttributeValue("subject");
                        //QString author = l.getAttributeValue("participant");
                        //QString notify = l.getAttributeValue("notify");
                        sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                        emit groupNewSubject(from, participant, notify, subject, subject_t);
                    }
                    else if (l.getTag() == "delete")
                    {
                        //groupRemoved(from);
                    }
                }
            }

            if (type == "participant")
            {
                ProtocolTreeNodeListIterator k(node.getChildren());

                while (k.hasNext())
                {
                    ProtocolTreeNode l = k.next().value();

                    if (l.getTag() == "add")
                    {
                        QString jid = l.getAttributeValue("jid");
                        if (!jid.isEmpty())
                        {
                            sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                            if (!from.contains(this->myJid) && jid!=this->myJid)
                                emit groupAddUser(from, jid);
                        }
                    }

                    if (l.getTag() == "remove")
                    {
                        QString jid = l.getAttributeValue("jid");
                        if (!jid.isEmpty())
                        {
                            sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                            emit groupRemoveUser(from, jid);
                        }
                    }

                    if (l.getTag() == "delete")
                    {
                        sendNotificationReceived(from, id, to, participant, type, ProtocolTreeNode());
                    }

                }
            }

        }


        if (tag == "message")
            parseMessageInitialTagAlreadyChecked(node);

        // Update counters
        if (tag != "message" && !pictureReceived && !synchronization)
            counters->increaseCounter(DataCounters::ProtocolBytes, node.getSize(), 0);

        return true;
    }

    return false;
}

/**
    Parses a node where the initial tag was already checked to be <message>.

    @param messageNode      ProtocolTreeNode object where its main tag is <message>.
*/
void Connection::parseMessageInitialTagAlreadyChecked(ProtocolTreeNode& messageNode)
{
    ChatMessageType msgType = Unknown;
    QString media;

    QString id = messageNode.getAttributeValue("id");
    QString attribute_t = messageNode.getAttributeValue("t");
    QString from = messageNode.getAttributeValue("from");
    QString author = messageNode.getAttributeValue("participant");
    QString typeAttribute = messageNode.getAttributeValue("type");


    if (typeAttribute == "text" || typeAttribute == "media")
    {
        ProtocolTreeNodeListIterator i(messageNode.getChildren());

        FMessage message;
        while (i.hasNext())
        {
            ProtocolTreeNode child = i.next().value();

            if (child.getTag() == "body")
            {
                // New message received

                Key k(from, false, id);
                message.setKey(k);
                message.setData(child.getData());
                message.remote_resource = author;
                message.setThumbImage("");
                message.type = FMessage::BodyMessage;
                if (!attribute_t.isEmpty())
                    message.timestamp = attribute_t.toLongLong() * 1000;

                msgType = (from.right(5) == "@s.us")
                        ? UserStatusUpdate : MessageReceived;
                sendMessageReceived(message);

            }
            else if (child.getTag() == "media")
            {
                // New mms received

                Key k(from, false, id);
                message.setKey(k);
                message.remote_resource = author;
                message.type = FMessage::MediaMessage;

                message.setMediaWAType(child.getAttributeValue("type"));
                message.media_url = child.getAttributeValue("url");
                message.media_caption  = child.getAttributeValue("caption");

                if (message.media_wa_type == FMessage::Location)
                {
                    message.media_name = child.getAttributeValue("name");
                    message.latitude = child.getAttributeValue("latitude").toLongLong();
                    message.longitude = child.getAttributeValue("latitude").toLongLong();
                }
                else
                    message.media_name = child.getAttributeValue("file");

                message.media_size = child.getAttributeValue("size").toLongLong();
                message.media_mime_type = child.getAttributeValue("mimetype");

                if (message.media_wa_type == FMessage::Video ||
                    message.media_wa_type == FMessage::Audio)
                    message.media_duration_seconds = child.getAttributeValue("duration").toInt();

                message.live = (child.getAttributeValue("origin") == "live");

                QString encoding = child.getAttributeValue("encoding");
                if (encoding == "text" || encoding == "raw")
                    message.setData(child.getData());

                msgType = MessageReceived;
                sendMessageReceived(message);
            }
            else if (child.getTag() == "notify")
            {
                QString xmlns = child.getAttributeValue("xmlns");
                if (xmlns == "urn:xmpp:whatsapp")
                {
                    QString notify_name = child.getAttributeValue("name");
                    message.notify_name = notify_name;
                }
            }
            else if (child.getTag() == "x")
            {
                QString xmlns = child.getAttributeValue("xmlns");
                if (xmlns == "jabber:x:event" && !id.isEmpty())
                {
                    Key k(from, true, id);
                    message = store.value(k);
                    if (message.key.id == id)
                    {
                        message.status = FMessage::ReceivedByServer;

                        msgType = (from == "s.us") ? UserStatusUpdate : MessageStatusUpdate;

                        // If this a receipt from a group message
                        // delete it from the FunStore
                        if (from.right(5) == "@g.us")
                            store.remove(k);
                    }
                }
                else if (xmlns == "jabber:x:delay")
                {
                    QString stamp = child.getAttributeValue("stamp");
                    message.timestamp = DateTimeUtilities::stampParser(stamp);
                }
            }
            else if (child.getTag() == "received")
            {
                QString receipt_type = child.getAttributeValue("type");
                Key k(from,true,id);
                message = store.value(k);
                if (message.key.id == id)
                {
                    message.status = (receipt_type == "played")
                            ? FMessage::Played
                            /* Whatsapp server sends the same "received" message twice:
                               once when received by the target and the second one when
                               it has been read (or at least displayed by the app) */
                            : (message.status == FMessage::ReceivedByTarget && Client::blueChecks)
                              ? FMessage::ReadByTarget
                              : FMessage::ReceivedByTarget;
                    msgType = (from == "s.us") ? Unknown : MessageStatusUpdate;

                    // Remove it from the store if it's not a voice message
                    // Or if it's a voice message already played
                    if ((message.live && receipt_type == "played") || !message.live)
                        store.remove(k);

                    // But restore it if still waiting for ReadByTarget
                    if (message.status == FMessage::ReceivedByTarget)
                        store.put(message);
                }
                if (receipt_type == "delivered" || receipt_type == "played" ||
                    receipt_type.isEmpty())
                {
                    // Delivery Receipt received
                    sendDeliveredReceiptAck(from,id,
                                            (receipt_type.isEmpty()
                                             ? "delivered"
                                             : receipt_type));
                }
            }
        }
        switch (msgType)
        {
            case MessageReceived:
                emit messageReceived(message);
                break;

            case MessageStatusUpdate:
                emit messageStatusUpdate(message);
                break;

            case UserStatusUpdate:
                emit userStatusUpdated(message);
                break;

            default:
                break;
        }

        // Increase data counters
        if (msgType == MessageReceived)
        {
            counters->increaseCounter(DataCounters::Messages, 1, 0);
            counters->increaseCounter(DataCounters::MessageBytes, messageNode.getSize(), 0);
        }
        else
            counters->increaseCounter(DataCounters::ProtocolBytes, messageNode.getSize(), 0);
    }
    else if (typeAttribute == "subject")
    {
        ProtocolTreeNode bodyNode = messageNode.getChild("body");
        ProtocolTreeNode notifyNode = messageNode.getChild("notify");

        QString authorName = notifyNode.getAttributeValue("name");
        QString newSubject = bodyNode.getDataString();

        if (!newSubject.isEmpty())
            emit groupNewSubject(from, author, authorName, newSubject, attribute_t);

        sendSubjectReceived(from, id);

    }

    //Obsolete?
    else if (typeAttribute == "error")
    {
        if (from.right(5) == "@g.us")
            emit groupError(from);
    }


    // Increase data counters for everything that was not a chat message
    if (typeAttribute != "chat")
        counters->increaseCounter(DataCounters::ProtocolBytes, messageNode.getSize(), 0);

}

/**
    Get the unixtime of the last node successfully read.

    @return     unixtime of the last node successfully read.
*/
qint64 Connection::getLastTreeReadTimestamp()
{
    return lastTreeRead;
}

/** ***********************************************************************
 ** Authentication methods
 **/

/**
    Sends the features supported by this client to the WhatsApp servers.

    So far this method sends the support of profile pictures.

    @return     number of bytes written to the socket.
*/
int Connection::sendFeatures()
{
    ProtocolTreeNode child1("privacy");
    ProtocolTreeNode child2("readreceipts");
    ProtocolTreeNode child3("groups_v2");
    ProtocolTreeNode child4("presence");
    ProtocolTreeNode node("stream:features");

    node.addChild(child1);
    node.addChild(child2);
    node.addChild(child3);
    node.addChild(child4);

    int bytes = out->write(node,false);
    return bytes;
}

/**
    Sends the authentication request.

    This method implements the WAUTH-1 WhatsApp protocol of authentication.

    @return     number of bytes written to the socket.
*/
int Connection::sendAuth()
{
    QByteArray data;
    if (nextChallenge.size() > 0)
        data = getAuthBlob(nextChallenge);

    AttributeList attrs;

    attrs.insert("passive", "false");
    attrs.insert("mechanism", "WAUTH-2");
    attrs.insert("user", user);

    ProtocolTreeNode node("auth", data);
    node.setAttributes(attrs);
    int bytes = out->write(node, false);
    if (nextChallenge.size() > 0)
        out->setCrypto(true);

    return bytes;
}

/**
    Constructs the authentication data to be sent.

    The authentication data is crypted with RC4 using the user password and the
    challenge data as salt.

    @param nonce    Challenge data.
    @return         Authentication blob encrypted.
*/
QByteArray Connection::getAuthBlob(QByteArray nonce)
{
    QList<QByteArray> keys = KeyStream::keyFromPasswordAndNonce(password, nonce);
    inputKey = new KeyStream(keys.at(2), keys.at(3), this);
    outputKey = new KeyStream(keys.at(0), keys.at(1), this);

    in->setInputKey(inputKey);
    out->setOutputKey(outputKey);

    QByteArray list(4, (char)0);

    list.append(user.toUtf8());
    list.append(nonce);

    qint64 totalSeconds = QDateTime::currentMSecsSinceEpoch() / 1000;
    list.append(QString::number(totalSeconds).toUtf8());

    outputKey->encodeMessage(list, 0, 4, list.length()-4);

    return list;
}

/**
    Reads the features from the node and the challenge data.

    @param bytes    Pointer to the number of bytes read to be updated.
    @return         New challenge data.
*/
QByteArray Connection::readFeaturesUntilChallengeOrSuccess(int *bytes)
{
    ProtocolTreeNode node;

    QByteArray data;
    bool moreNodes;
    bool server_supports_receipts_acks = false;
    int server_properties_version = -1;

    while ((moreNodes = in->nextTree(node)))
    {
        *bytes += node.getSize();

        if (node.getTag() == "stream:features")
        {
            ProtocolTreeNode receiptAcksNode = node.getChild("receipt_acks");
            ProtocolTreeNode propsNode = node.getChild("props");

            // ToDo: What can we do with this?
            server_supports_receipts_acks = receiptAcksNode.getTag() == "receipt_acks";

            if (propsNode.getTag() == "props")
            {
                // ToDo: What can we do with this?
                server_properties_version = propsNode.getAttributeValue("version").toInt();
            }
        }

        if (node.getTag() == "challenge")
        {
            data = node.getData();
            Utilities::logData("Challenge: (" + QString::number(data.length()) + ") " +
                               QString::fromLatin1(data.toHex()));

            return data;
        }

        if (node.getTag() == "success")
        {
            parseSuccessNode(node);
            return data;
        }
    }

    return data;
}

/**
    Sends authentication response.

    @param challengeData    Authentication challenge data (nonce/salt).
    @return                 number of bytes written to the socket.
*/
int Connection::sendResponse(QByteArray challengeData)
{
    QByteArray authBlob = getAuthBlob(challengeData);

    AttributeList attrs;

    ProtocolTreeNode node("response", authBlob);
    node.setAttributes(attrs);
    int bytes = out->write(node, false);
    out->setCrypto(true);

    return bytes;
}

/**
    Read the authentication success node and parse it.

    @return     number of bytes read from the socket (node size).
*/
int Connection::readSuccess()
{
    ProtocolTreeNode node;

    in->nextTree(node);
    parseSuccessNode(node);

    return node.getSize();
}

/**
    Parses the authentication success node.

    This method retrieves all the account information.  It also updates the
    next challenge data.

    @param node     ProtocolTreeNode object with the authentication success node.
*/
void Connection::parseSuccessNode(ProtocolTreeNode& node)
{
    if (node.getTag() == "failure")
        throw LoginException("Login failed");

    if (node.getTag() != "success")
        throw LoginException("Login success required");

    // This has to be converted to a date object
    expiration = node.getAttributeValue("expiration");
    creation = node.getAttributeValue("creation");
    kind = node.getAttributeValue("kind");
    accountstatus = node.getAttributeValue("status");

    nextChallenge = node.getData();
}

/** ***********************************************************************
 ** Message handling methods
 **/

/**
    Sends a message

    @param message      FMessage object containing the message.
*/
void Connection::sendMessage(FMessage& message)
{
    switch (message.type)
    {
        case FMessage::ComposingMessage:
            sendComposing(message);
            break;

        case FMessage::PausedMessage:
            sendPaused(message);
            break;

        case FMessage::BodyMessage:
            sendMessageWithBody(message);
            message.status = FMessage::SentByClient;
            // emit messageStatusUpdate(message);
            break;

        case FMessage::MediaMessage:
            sendMessageWithMedia(message);
            break;

        case FMessage::RequestMediaMessage:
            requestMessageWithMedia(message);

        default:
            break;
    }
}

/**
    Sends a text message.

    @param message      FMessage object containing the text message.
*/
void Connection::sendMessageWithBody(FMessage& message)
{
    // Add it to the store
    store.put(message);

    ProtocolTreeNode bodyNode("body", message.data);
    ProtocolTreeNode messageNode;

    messageNode = getMessageNode(message, bodyNode);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::Messages, 0, 1);
    counters->increaseCounter(DataCounters::MessageBytes, 0, bytes);
}

/**
    Sends a request to send a multimedia message.

    To send a multimedia message to another user an upload request has to be sent
    first.  This method sends such request.

    @param message      FMessage object containing the multimedia message.
*/
void Connection::requestMessageWithMedia(FMessage &message)
{
    Utilities::logData("key: " + message.key.remote_jid + " : " + message.key.id);

    // Add it to the store
    store.put(message);

    AttributeList attrs;

    ProtocolTreeNode mediaNode("media");
    attrs.insert("hash", message.data);
    attrs.insert("type", message.getMediaWAType());
    attrs.insert("size", QString::number(message.media_size));
    if (message.live)
        attrs.insert("origin","live");

    mediaNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id", message.key.id);
    attrs.insert("type","set");
    attrs.insert("to",domain);
    attrs.insert("xmlns","w:m");
    iqNode.setAttributes(attrs);
    iqNode.addChild(mediaNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a multimedia message.

    @param message      FMessage object containing the multimedia message.
*/
void Connection::sendMessageWithMedia(FMessage& message)
{
    // Add it to the store
    store.put(message);

    // Global multimedia messages attributes
    AttributeList attrs;
    attrs.insert("xmlns","urn:xmpp:whatsapp:mms");
    attrs.insert("type" ,message.getMediaWAType());

    if (!message.media_name.isEmpty() && !message.media_url.isEmpty() &&
        message.media_size > 0)
    {
        attrs.insert("file", message.media_name);
        attrs.insert("size", QString::number(message.media_size));
        attrs.insert("url", message.media_url);
        if(message.media_caption.length()>0)
            attrs.insert("caption", message.media_caption);
        if (message.live)
            attrs.insert("origin","live");

        if (message.media_wa_type == FMessage::Audio ||
            message.media_wa_type == FMessage::Video)
        {
            attrs.insert("duration", QString::number(message.media_duration_seconds));
            attrs.insert("seconds", QString::number(message.media_duration_seconds));
        }
    }

    if (message.media_wa_type == FMessage::Contact && !message.media_name.isEmpty())
    {
        /**
         ** @todo   Send contacts
         **/
    }
    else
    {
        if (message.data.size() > 0)
            attrs.insert("encoding","raw");

        ProtocolTreeNode mediaNode("media", message.data);
        mediaNode.setAttributes(attrs);

        ProtocolTreeNode messageNode = getMessageNode(message, mediaNode);

        int bytes = out->write(messageNode);
        counters->increaseCounter(DataCounters::Messages, 0, 1);
        counters->increaseCounter(DataCounters::MessageBytes, 0, bytes);
    }
}

/**
    Constructs a message node.

    @param message      FMessage object containing the message.
    @param child        ProtocolTreeNode object with the message data
    @return             ProtocolTreeNode object containing the message node.
*/
ProtocolTreeNode Connection::getMessageNode(FMessage& message, ProtocolTreeNode& child)
{
    ProtocolTreeNode messageNode("message");
    AttributeList attrs;
    attrs.insert("id",message.key.id);
    attrs.insert("type", child.getTag() == "body" ? "text" : "media");
    attrs.insert("to",message.key.remote_jid);
    messageNode.setAttributes(attrs);

    Utilities::logData("Message ID " + message.key.id);

    messageNode.addChild(child);

    return messageNode;
}

/**
    Send a message received acknowledgement.

    @param message      FMessage object containing the message to be acknowledged.
    @param type         message type (only need "read" string for read messages)
*/
void Connection::sendMessageReceived(FMessage &message)
{
    AttributeList attrs;

    ProtocolTreeNode messageNode("receipt");
    attrs.insert("to",message.key.remote_jid);
    attrs.insert("id",message.key.id);
    if (!message.remote_resource.isEmpty()) {
        attrs.insert("participant", message.remote_resource);
    }

    messageNode.setAttributes(attrs);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Send a message read acknowledgement.

    @param message      FMessage object containing the message to be acknowledged.
*/
void Connection::sendMessageRead(FMessage& message)
{
    AttributeList attrs;

    ProtocolTreeNode messageNode("receipt");
    attrs.insert("to",message.key.remote_jid);
    attrs.insert("id",message.key.id);
    attrs.insert("type","read");
    if (!message.remote_resource.isEmpty()) {
        attrs.insert("participant", message.remote_resource);
    }

    messageNode.setAttributes(attrs);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);

    store.remove(message.key);

    message.status = FMessage::ReceivedByTarget;
    store.put(message);
}

/**
    Send a notification received acknowledgement.

    Notification nodes usually contain profile pictures change notifications.

    @param message      FMessage object containing the message to be acknowledged.
    // fix it!
*/
void Connection::sendNotificationReceived(QString to, QString id, QString from, QString participant, QString type, ProtocolTreeNode childNode)
{
    AttributeList attrs;

    ProtocolTreeNode ackNode("ack");
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("class","notification");
    attrs.insert("id",id);
    attrs.insert("type",type);
    if (!participant.isEmpty()) {
        attrs.insert("participant", participant);
    }
    if (!from.isEmpty()) {
        attrs.insert("from", from);
    }
    ackNode.setAttributes(attrs);
    if (!childNode.getTag().isEmpty()) {
        ackNode.addChild(childNode);
    }

    int bytes = out->write(ackNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Constructs a receipt acknowledgement node.

    @param node         Node where the receipt acknowledge will be stored.
    @param to           Destination jid.
    @param id           Id of the receipt received.
    @param receiptType  Type of the receipt. Example: "delivered".
*/
void Connection::getReceiptAck(ProtocolTreeNode& node, QString to, QString id,
                               QString receiptType)
{
    ProtocolTreeNode ackNode("ack");
    AttributeList attrs;
    attrs.insert("type", receiptType.isEmpty()? "delivered" : receiptType);
    ackNode.setAttributes(attrs);

    node.setTag("message");
    node.addChild(ackNode);
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("type","chat");
    attrs.insert("id",id);
    node.setAttributes(attrs);
}

/**
    Sends a receipt acknowledging a delivered message notification received.

    @param to           Destination jid.
    @param id           Id of the receipt received.
    @param type         Type of the ack (delivered, played)
*/
void Connection::sendDeliveredReceiptAck(QString to, QString id, QString type, QString participant)
{
    ProtocolTreeNode ackNode("ack");
    AttributeList attrs;
    attrs.insert("class","receipt");
    attrs.insert("type",type.isEmpty() ? "delivery" : type);
    attrs.insert("id",id);
    if (!to.contains("s.us")) attrs.insert("to",to);
    if (participant!="") attrs.insert("participant", participant);
    ackNode.setAttributes(attrs);

    int bytes = out->write(ackNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/** ***********************************************************************
 ** User handling methods
 **/

/**
    Sends a query to request the time when a user was last seen online.

    @param jid           Destination jid.
*/
void Connection::sendQueryLastOnline(QString jid)
{
    AttributeList attrs;

    QString id = makeId("last_");

    ProtocolTreeNode queryNode("query");
    attrs.insert("xmlns","jabber:iq:last");
    queryNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("to",jid);
    iqNode.setAttributes(attrs);
    iqNode.addChild(queryNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a contact synchronization request.

    @param jid           Destination jid.
*/
void Connection::sendSyncContacts(QStringList numbers)
{
    QString id = makeId("sync_");

    AttributeList attrs;

    ProtocolTreeNode syncNode("sync");
    attrs.clear();
    attrs.insert("context", "background");
    attrs.insert("index", "0");
    attrs.insert("mode", "delta");
    attrs.insert("request", "all");
    attrs.insert("sid", "sync");
    syncNode.setAttributes(attrs);

    foreach (QString number, numbers) {
        ProtocolTreeNode userNode("user");
        userNode.setData(number.split("@").first().toUtf8());
        syncNode.addChild(userNode);
    }

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id", id);
    attrs.insert("type", "get");
    attrs.insert("to", myJid);
    attrs.insert("xmlns", "urn:xmpp:whatsapp:sync");
    iqNode.setAttributes(attrs);
    iqNode.addChild(syncNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::SyncBytes, 0, bytes);
}

/**
    Sends a query to get the current status of a user.

    @param jid           Destination jid.
*/
void Connection::sendGetStatus(QStringList jids)
{
    QString id = makeId("getstatus_");

    AttributeList attrs;

    ProtocolTreeNode statusNode("status");
    attrs.clear();
    statusNode.setAttributes(attrs);

    foreach (QString jid, jids) {
        if (jid.contains("@"))
        {
            ProtocolTreeNode userNode("user");
            attrs.clear();
            attrs.insert("jid", jid);
            userNode.setAttributes(attrs);
            statusNode.addChild(userNode);
        }
    }

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id", id);
    attrs.insert("to", "s.whatsapp.net");
    attrs.insert("type", "get");
    attrs.insert("xmlns", "status");
    iqNode.setAttributes(attrs);
    iqNode.addChild(statusNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a query to request a subscription to a user.

    Subscriptions allow servers to send user online status changes in real time
    to this client.

    @param jid           Destination jid.
*/
void Connection::sendPresenceSubscriptionRequest(QString jid)
{
    ProtocolTreeNode presenceNode("presence");

    AttributeList attrs;
    attrs.insert("to", jid);
    attrs.insert("type", "subscribe");
    presenceNode.setAttributes(attrs);

    int bytes = out->write(presenceNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a query to request a subscription remove to a user.

    Subscriptions allow servers to send user online status changes in real time
    to this client.

    @param jid           Destination jid.
*/
void Connection::sendUnsubscribeHim(QString jid)
{
    ProtocolTreeNode presenceNode("presence");

    AttributeList attrs;
    attrs.insert("to", jid);
    attrs.insert("type", "unsubscribed");
    presenceNode.setAttributes(attrs);

    int bytes = out->write(presenceNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/** ***********************************************************************
 ** Picture handling methods
 **/

/**
    Sends a query to request to get the current profile picture of a user.

    If the profile picture of a user that is currently stored in the servers
    have the same photoId than the request then the profile picture is not
    sent again by the servers.

    @param jid              Destination jid.
    @param expectedPhotoId  The expected photo Id. If this string is empty then
                            a new photo will always be received.  If the photo Id
                            is the same that the servers have they won't send the same
                            photo again.
    @param largeFormat      True to request the full resolution picture.  False to
                            request just a preview.
*/
void Connection::sendGetPhoto(QString jid, QString expectedPhotoId, bool largeFormat)
{
    AttributeList attrs;

    QString id = makeId("get_photo_");

    ProtocolTreeNode pictureNode("picture");

    if (!largeFormat)
        attrs.insert("type", "preview");
    else
        attrs.insert("type", "image");

    if (!expectedPhotoId.isEmpty() && expectedPhotoId != "abook")
        attrs.insert("id", expectedPhotoId);

    pictureNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("to",jid);
    attrs.insert("xmlns", "w:profile:picture");
    iqNode.setAttributes(attrs);
    iqNode.addChild(pictureNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to set a photo.

    The photo can be the user's own profile photo (if the jid is the same as the
    user's jid) or a group icon (the user has to belong to that group).

    Currently the thumbnail is not supported. It is being ignored and not sent to
    the servers.

    @param jid          Destination jid.
    @param imageBytes   Image bytes. The image has to be in JPEG format.
    @param thumbBytes   Thumbnail image bytes. The thumbnail has to be in JPEG format.
*/
void Connection::sendSetPhoto(QString jid, QByteArray imageBytes, QByteArray thumbBytes)
{
    Q_UNUSED(thumbBytes);

    AttributeList attrs;

    QString id = makeId("set_photo_");

    ProtocolTreeNode pictureNode("picture");
    // attrs.insert("type", "image");
    pictureNode.setAttributes(attrs);
    pictureNode.setData(imageBytes);

    attrs.clear();
    ProtocolTreeNode thumbNode("picture");
    attrs.insert("type","preview");
    thumbNode.setData(thumbBytes);
    thumbNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to",jid);
    attrs.insert("xmlns", "w:profile:picture");
    iqNode.setAttributes(attrs);
    iqNode.addChild(pictureNode);
    iqNode.addChild(thumbNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProfileBytes, 0, bytes);
}

/**
    Sends a request to get the photo Ids from a list of jids.

    @param jids         QStringList containing the jids.
*/
void Connection::sendGetPhotoIds(QStringList jids)
{
    AttributeList attrs;

    QString id = makeId("get_photo_id_");

    ProtocolTreeNode listNode("list");
    listNode.setAttributes(attrs);

    foreach (QString jid, jids)
    {
        ProtocolTreeNode userNode("user");
        attrs.insert("jid", jid);
        userNode.setAttributes(attrs);
        listNode.addChild(userNode);
    }

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("xmlns","w:profile:picture");
    iqNode.setAttributes(attrs);
    iqNode.addChild(listNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a notification that a voice note was played

    @param message      Voice note that was played by the user
*/
void Connection::sendVoiceNotePlayed(FMessage message)
{
    AttributeList attrs;

    ProtocolTreeNode receivedNode("received");
    attrs.clear();
    attrs.insert("xmlns","urn:xmpp:receipts");
    attrs.insert("type","played");
    receivedNode.setAttributes(attrs);
    ProtocolTreeNode messageNode = getMessageNode(message, receivedNode);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}


/** ***********************************************************************
 ** Typing status handling methods
 **/

/**
    Sends a notification that the user is composing a message.

    @param to   Destination jid.
*/
void Connection::sendComposing(FMessage message)
{
    AttributeList attrs;

    ProtocolTreeNode composingNode("composing");
    attrs.insert("xmlns","http://jabber.org/protocol/chatstates");
    if (!message.media_wa_type >=  0)
        attrs.insert("media",message.getMediaWAType());
    composingNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("chatstate");
    attrs.clear();
    attrs.insert("to",message.key.remote_jid);
    messageNode.setAttributes(attrs);
    messageNode.addChild(composingNode);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a notification that the user has stopped typing a message.

    @param to   Destination jid.
*/
void Connection::sendPaused(FMessage message)
{
    AttributeList attrs;

    ProtocolTreeNode pausedNode("paused");
    attrs.insert("xmlns","http://jabber.org/protocol/chatstates");
    pausedNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("chatstate");
    attrs.clear();
    attrs.insert("to",message.key.remote_jid);
    messageNode.setAttributes(attrs);
    messageNode.addChild(pausedNode);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}


/** ***********************************************************************
 ** Group handling
 **/

/**
    Sends a request to create a group.

    @param subject  Group name or subject.
    @param id       Id of the request.
*/
void Connection::sendCreateGroupChat(QString subject, QString id)
{
    ProtocolTreeNode groupNode("group");

    AttributeList attrs;
    attrs.insert("action", "create");
    attrs.insert("subject", subject);
    groupNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to","g.us");
    attrs.insert("xmlns", "w:g");
    iqNode.setAttributes(attrs);
    iqNode.addChild(groupNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to add participants to a group.

    @param gjid             Group jid.
    @param participants     QStringList containing the jids of the participants
                            to add.
*/
void Connection::sendAddParticipants(QString gjid, QStringList participants)
{
    QString id = makeId("add_group_participants_");

    sendVerbParticipants(gjid, participants, id, "add");
}

/**
    Sends a request to remove participants from a group.

    @param gjid             Group jid.
    @param participants     QStringList containing the jids of the participants
                            to remove.
*/
void Connection::sendRemoveParticipants(QString gjid, QStringList participants)
{
    QString id = makeId("remove_group_participants_");

    sendVerbParticipants(gjid, participants, id, "remove");
}

/**
    Sends a request with participants to a group.

    @param gjid             Group jid.
    @param participants     QStringList containing the jids of the participants
                            to remove.
    @param id               Id of the request.
    @param innerTag         Tag of the inner node.  Usually contains the action
                            to be applied to the participants, like "add" or
                            "remove".
*/
void Connection::sendVerbParticipants(QString gjid, QStringList participants,
                                      QString id, QString innerTag)
{
    ProtocolTreeNode innerNode(innerTag);

    foreach (QString jid, participants)
    {
        ProtocolTreeNode participantNode("participant");
        AttributeList attrs;
        attrs.insert("jid", jid);
        participantNode.setAttributes(attrs);
        innerNode.addChild(participantNode);
    }

    AttributeList attrs;
    innerNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to",gjid);
    attrs.insert("xmlns", "w:g");
    iqNode.setAttributes(attrs);
    iqNode.addChild(innerNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to retrieve the participants list of a group

    @param gjid             Group jid.
*/
void Connection::sendGetParticipants(QString gjid)
{
    ProtocolTreeNode listNode("list");

    QString id = makeId("get_participants_");

    AttributeList attrs;
    listNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("to",gjid);
    attrs.insert("xmlns", "w:g");
    iqNode.setAttributes(attrs);
    iqNode.addChild(listNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to retrieve the participants list of all groups
*/
void Connection::updateGroupChats()
{
    QString id = makeId("get_groups_");
    sendGetGroups(id,"participating");
}

/**
    Sends a request to all groups.

    This method retrieves an specific information from all groups. The type argument
    indicates the information to be retrieved.

    @param id       Id of the request.
    @param type     Type of the operation. Example: "participating" to get the list
                    of all participants of every group the user belongs to.
*/
void Connection::sendGetGroups(QString id, QString type)
{
    AttributeList attrs;

    ProtocolTreeNode listNode("list");
    attrs.insert("type",type);
    listNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("to","g.us");
    attrs.insert("xmlns","w:g");
    iqNode.setAttributes(attrs);
    iqNode.addChild(listNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to change/set a group subject.

    @param gjid     Group jid.
    @param subject  New subject/name for the group
*/
void Connection::sendSetGroupSubject(QString gjid, QString subject)
{
    QString id = makeId("set_group_subject_");

    AttributeList attrs;

    ProtocolTreeNode subjectNode("subject");
    attrs.insert("value",subject);
    subjectNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to",gjid);
    attrs.insert("xmlns","w:g");
    iqNode.setAttributes(attrs);
    iqNode.addChild(subjectNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to leave a group.

    @param gjid     Group jid.
*/
void Connection::sendLeaveGroup(QString gjid)
{
    QString id = makeId("leave_group_");

    AttributeList attrs;

    ProtocolTreeNode groupNode("group");
    attrs.insert("id",gjid);
    groupNode.setAttributes(attrs);

    ProtocolTreeNode leaveNode("leave");
    leaveNode.addChild(groupNode);

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to","g.us");
    attrs.insert("xmlns","w:g");
    iqNode.setAttributes(attrs);
    iqNode.addChild(leaveNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a notification that a group subject was changed.

    @param to   Group jid.
    @param id   Id of the notification received.
*/
void Connection::sendSubjectReceived(QString to, QString id)
{
    AttributeList attrs;

    ProtocolTreeNode receivedNode("received");
    attrs.insert("xmlns","urn:xmpp:receipts");
    receivedNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("message");
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("type","subject");
    attrs.insert("id",id);
    messageNode.setAttributes(attrs);
    messageNode.addChild(receivedNode);

    int bytes = out->write(messageNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/** ***********************************************************************
 ** Privacy list methods
 **/

/**
    Sends a request to get the privacy list.
    The privacy list is the same than the blocked contacts list.
*/
void Connection::sendGetPrivacyList()
{
    QString id = makeId("privacylist_");

    AttributeList attrs;

    ProtocolTreeNode listNode("list");
    attrs.insert("name","default");
    listNode.setAttributes(attrs);

    ProtocolTreeNode queryNode("query");
    queryNode.addChild(listNode);

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("xmlns","jabber:iq:privacy");
    iqNode.setAttributes(attrs);
    iqNode.addChild(queryNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a request to set the privacy blocked list

    @param jidList  List of JIDs that are blocked.
*/
void Connection::sendSetPrivacyBlockedList(QStringList jidList)
{
    QString id = makeId("privacy_");

    AttributeList attrs;

    ProtocolTreeNode listNode("list");
    attrs.insert("name","default");
    listNode.setAttributes(attrs);

    int order = 1;
    foreach(QString jid, jidList)
    {
        ProtocolTreeNode itemNode("item");
        attrs.clear();
        attrs.insert("type","jid");
        attrs.insert("value",jid);
        attrs.insert("action","deny");
        attrs.insert("order",QString::number(order++));
        itemNode.setAttributes(attrs);
        listNode.addChild(itemNode);
    }

    ProtocolTreeNode queryNode("query");
    queryNode.addChild(listNode);

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("xmlns","jabber:iq:privacy");
    iqNode.setAttributes(attrs);
    iqNode.addChild(queryNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/** ***********************************************************************
 ** General methods
 **/

/**
    Sends a no operation (ping) to the network.
*/
void Connection::sendNop()
{

    ProtocolTreeNode empty;

    int bytes = out->write(empty);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a ping request to the network.
*/
void Connection::sendPing()
{
    QString id = makeId("ping_");

    AttributeList attrs;
    ProtocolTreeNode pingNode("ping");

    ProtocolTreeNode iqNode("iq");
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("xmlns","w:p");
    iqNode.setAttributes(attrs);
    iqNode.addChild(pingNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends a ping acknowledge (pong) to the network.

    @param id   Id of the ping received.
*/
void Connection::sendPong(QString id)
{
    AttributeList attrs;
    ProtocolTreeNode iqNode("iq");

    attrs.insert("id",id);
    attrs.insert("type","result");
    attrs.insert("to",domain);
    iqNode.setAttributes(attrs);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Constructs an id.

    @param prexix   String containing the prefix of the id.
*/
QString Connection::makeId(QString prefix)
{
    return prefix + QString::number(++iqid,16);
}

/**
    Changes the user name or alias.

    @param push_name    New user name or alias.
*/
void Connection::setNewUserName(QString push_name)
{
    this->push_name = push_name;
    sendAvailableForChat();
}

/**
    Sends the client configuration.

    @param platform     Platform that this client is running on.
*/
void Connection::sendClientConfig(QString platform)
{
    QSystemInfo systemInfo(this);

#ifdef Q_WS_SCRATCHBOX
    QString language = "en";
    QString country = "US";
#else
    QString language = systemInfo.currentLanguage();
    QString country = systemInfo.currentCountryCode();
#endif

    QString id = makeId("config_");

    AttributeList attrs;

    ProtocolTreeNode configNode("config");
    attrs.insert("xmlns","urn:xmpp:whatsapp:push");
    attrs.insert("platform", platform);
    attrs.insert("lg", language.isEmpty() ? "zz" : language);
    attrs.insert("lc", country.isEmpty() ?  "ZZ" : country);
    configNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to",domain);
    iqNode.setAttributes(attrs);
    iqNode.addChild(configNode);

    int bytes = out->write(iqNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Sends notification that this client is available for chat (online).
*/
void Connection::sendAvailableForChat()
{
    ProtocolTreeNode presenceNode("presence");

    AttributeList attrs;
    attrs.insert("name",push_name);
    presenceNode.setAttributes(attrs);

    int bytes = out->write(presenceNode);
    counters->increaseCounter(DataCounters::ProtocolBytes, 0, bytes);
}

/**
    Connection timeout handler
*/
void Connection::connectionTimeout()
{
    connTimeout.stop();
    emit timeout();
}

