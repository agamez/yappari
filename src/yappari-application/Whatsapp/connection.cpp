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

#include <QRegExp>
#include <QUuid>
#include <QTime>

#include "Whatsapp/util/qtmd5digest.h"
#include "Whatsapp/util/utilities.h"
#include "Whatsapp/util/datetimeutilities.h"
#include "Whatsapp/protocoltreenodelistiterator.h"

#include "globalconstants.h"

#include "connection.h"

// QStringList Connection::dictionary;
FunStore Connection::store;

Connection::Connection(QTcpSocket *socket, QString domain, QString resource,
                       QString user, QString push_name, QByteArray password,
                       QObject *parent) : QObject(parent)
{

    /*
     * This is the dictionary Whatsapp uses to compress its data
     * so the packets are really tiny
     */

    dictionary
        << NULL << NULL << NULL << NULL << NULL << "account" << "ack" << "action" << "active" << "add" << "after" << "ib" << "all" << "allow" << "apple" << "audio" << "auth" << "author" << "available" << "bad-protocol" << "bad-request" << "before" << "Bell.caf" << "body" << "Boing.caf" << "cancel" << "category" << "challenge" << "chat" << "clean" << "code" << "composing" << "config" << "conflict" << "contacts" << "count" << "create" << "creation" << "default" << "delay" << "delete" << "delivered" << "deny" << "digest" << "DIGEST-MD5-1" << "DIGEST-MD5-2" << "dirty" << "elapsed" << "broadcast" << "enable" << "encoding" << "duplicate" << "error" << "event" << "expiration" << "expired" << "fail" << "failure" << "false" << "favorites" << "feature" << "features" << "field" << "first" << "free" << "from" << "g.us" << "get" << "Glass.caf" << "google" << "group" << "groups" << "g_notify" << "g_sound" << "Harp.caf" << "http://etherx.jabber.org/streams" << "http://jabber.org/protocol/chatstates" << "id" << "image" << "img" << "inactive" << "index" << "internal-server-error" << "invalid-mechanism" << "ip" << "iq" << "item" << "item-not-found" << "user-not-found" << "jabber:iq:last" << "jabber:iq:privacy" << "jabber:x:delay" << "jabber:x:event" << "jid" << "jid-malformed" << "kind" << "last" << "latitude" << "lc" << "leave" << "leave-all" << "lg" << "list" << "location" << "longitude" << "max" << "max_groups" << "max_participants" << "max_subject" << "mechanism" << "media" << "message" << "message_acks" << "method" << "microsoft" << "missing" << "modify" << "mute" << "name" << "nokia" << "none" << "not-acceptable" << "not-allowed" << "not-authorized" << "notification" << "notify" << "off" << "offline" << "order" << "owner" << "owning" << "paid" << "participant" << "participants" << "participating" << "password" << "paused" << "picture" << "pin" << "ping" << "platform" << "pop_mean_time" << "pop_plus_minus" << "port" << "presence" << "preview" << "probe" << "proceed" << "prop" << "props" << "p_o" << "p_t" << "query" << "raw" << "reason" << "receipt" << "receipt_acks" << "received" << "registration" << "relay" << "remote-server-timeout" << "remove" << "Replaced by new connection" << "request" << "required" << "resource" << "resource-constraint" << "response" << "result" << "retry" << "rim" << "s.whatsapp.net" << "s.us" << "seconds" << "server" << "server-error" << "service-unavailable" << "set" << "show" << "sid" << "silent" << "sound" << "stamp" << "unsubscribe" << "stat" << "status" << "stream:error" << "stream:features" << "subject" << "subscribe" << "success" << "sync" << "system-shutdown" << "s_o" << "s_t" << "t" << "text" << "timeout" << "TimePassing.caf" << "timestamp" << "to" << "Tri-tone.caf" << "true" << "type" << "unavailable" << "uri" << "url" << "urn:ietf:params:xml:ns:xmpp-sasl" << "urn:ietf:params:xml:ns:xmpp-stanzas" << "urn:ietf:params:xml:ns:xmpp-streams" << "urn:xmpp:delay" << "urn:xmpp:ping" << "urn:xmpp:receipts" << "urn:xmpp:whatsapp" << "urn:xmpp:whatsapp:account" << "urn:xmpp:whatsapp:dirty" << "urn:xmpp:whatsapp:mms" << "urn:xmpp:whatsapp:push" << "user" << "username" << "value" << "vcard" << "version" << "video" << "w" << "w:g" << "w:p" << "w:p:r" << "w:profile:picture" << "wait" << "x" << "xml-not-well-formed" << "xmlns" << "xmlns:stream" << "Xylophone.caf" << "1" << "WAUTH-1" ;

    this->socket = socket;
    this->user = user;
    this->domain = domain;
    this->resource = resource;
    this->push_name = push_name;
    this->password = password;
    this->iqid = 0;
    this->myJid = user + "@" + JID_DOMAIN;

    this->out = new BinTreeNodeWriter(socket,dictionary,this);
    this->in = new BinTreeNodeReader(socket,dictionary,this);
}

// Destructor needed to delete out

Connection::~Connection()
{
}

void Connection::login(QByteArray nextChallenge)
{
    this->nextChallenge = nextChallenge;

    out->streamStart(domain,resource);

    sendFeatures();
    sendAuth();
    in->readStreamStart();
    QByteArray challengeData = readFeaturesUntilChallengeOrSuccess();
    if (challengeData.size() > 0)
    {
        sendResponse(challengeData);
        readSuccess();
    }

    // Successful login at this point

    sendClientConfig("none");
    sendAvailableForChat();
}

void Connection::sendMessage(FMessage& message)
{
    Utilities::logData("Connection::sendMessage()");

    switch (message.type)
    {
        case FMessage::ComposingMessage:
            sendComposing(message.key.remote_jid);
            break;

        case FMessage::PausedMessage:
            sendPaused(message.key.remote_jid);
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

void Connection::sendMessageWithBody(FMessage& message)
{
    // Add it to the store
    store.put(message);

    ProtocolTreeNode bodyNode("body", message.data);
    ProtocolTreeNode messageNode;

    messageNode = getMessageNode(message, bodyNode);

    out->write(messageNode);

}

void Connection::requestMessageWithMedia(FMessage &message)
{
    Utilities::logData("key: " + message.key.remote_jid + " : " + message.key.id);

    // Add it to the store
    store.put(message);

    AttributeList attrs;

    ProtocolTreeNode mediaNode("media");
    attrs.insert("xmlns","w:m");
    attrs.insert("hash", message.data);
    attrs.insert("type", message.getMediaWAType());
    attrs.insert("size", QString::number(message.media_size));
    mediaNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id", message.key.id);
    attrs.insert("type","set");
    attrs.insert("to",domain);
    iqNode.setAttributes(attrs);
    iqNode.addChild(mediaNode);

    out->write(iqNode);

}

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
    }

    if (message.media_wa_type == FMessage::Contact && !message.media_name.isEmpty())
    {

    }
    else
    {
        if (message.data.size() > 0)
            attrs.insert("encoding","raw");

        ProtocolTreeNode mediaNode("media", message.data);
        mediaNode.setAttributes(attrs);

        ProtocolTreeNode messageNode = getMessageNode(message, mediaNode);

        out->write(messageNode);
    }
}

ProtocolTreeNode Connection::getMessageNode(FMessage& message, ProtocolTreeNode& child)
{
    ProtocolTreeNode serverNode("server");

    ProtocolTreeNode xNode("x");
    AttributeList attrs;
    attrs.insert("xmlns","jabber:x:event");
    xNode.setAttributes(attrs);
    xNode.addChild(serverNode);

    attrs.clear();
    attrs.insert("id",message.key.id);
    attrs.insert("type","chat");
    attrs.insert("to",message.key.remote_jid);

    Utilities::logData("Message ID " + message.key.id);

    ProtocolTreeNode messageNode("message");
    messageNode.setAttributes(attrs);
    messageNode.addChild(child);
    messageNode.addChild(xNode);

    return messageNode;
}

void Connection::sendFeatures()
{
    ProtocolTreeNode child("receipt_acks");

    AttributeList attrs;

    ProtocolTreeNode child2("w:profile:picture");
    attrs.insert("type","all");
    child2.setAttributes(attrs);

    attrs.clear();

    ProtocolTreeNode child3("status");


    ProtocolTreeNode node("stream:features");
    node.addChild(child);
    node.addChild(child2);
    node.addChild(child3);

    out->write(node,false);
}

void Connection::sendAuth()
{
    QByteArray data;
    if (nextChallenge.size() > 0)
        data = getAuthBlob(nextChallenge);

    AttributeList attrs;

    attrs.insert("xmlns","urn:ietf:params:xml:ns:xmpp-sasl");
    attrs.insert("mechanism","WAUTH-1");
    attrs.insert("user",user);

    ProtocolTreeNode node("auth", data);
    node.setAttributes(attrs);
    out->write(node, false);
    if (nextChallenge.size() > 0)
        out->setCrypto(true);
}

QByteArray Connection::getAuthBlob(QByteArray nonce)
{
    QByteArray key = KeyStream::keyFromPasswordAndNonce(password,nonce);
    inputKey = new KeyStream(key, this);
    outputKey = new KeyStream(key, this);

    in->setInputKey(inputKey);
    out->setOutputKey(outputKey);

    QByteArray list;
    for (int i = 0; i < 4; i++)
        list.append(QChar(0));

    list.append(user.toUtf8());
    list.append(nonce);

    qint64 totalSeconds = QDateTime::currentMSecsSinceEpoch() / 1000;
    list.append(QString::number(totalSeconds).toUtf8());

    outputKey->encodeMessage(list, 0, 4, list.length()-4);

    return list;
}


QByteArray Connection::readFeaturesUntilChallengeOrSuccess()
{
    ProtocolTreeNode node;

    QByteArray data;
    bool moreNodes;
    bool server_supports_receipts_acks = false;
    int server_properties_version = -1;

    while ((moreNodes = in->nextTree(node)))
    {
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

void Connection::sendResponse(QByteArray challengeData)
{
    QByteArray authBlob = getAuthBlob(challengeData);

    AttributeList attrs;

    attrs.insert("xmlns","urn:ietf:params:xml:ns:xmpp-sasl");
    ProtocolTreeNode node("response", authBlob);
    node.setAttributes(attrs);
    out->write(node, false);
    out->setCrypto(true);
}

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

void Connection::readSuccess()
{
    ProtocolTreeNode node;

    in->nextTree(node);
    parseSuccessNode(node);
}

void Connection::sendAvailableForChat()
{
    ProtocolTreeNode presenceNode("presence");

    AttributeList attrs;
    attrs.insert("name",push_name);
    presenceNode.setAttributes(attrs);

    out->write(presenceNode);
}

bool Connection::read()
{
    ProtocolTreeNode node;

    if (in->nextTree(node))
    {
        lastTreeRead = QDateTime::currentMSecsSinceEpoch();
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
                        emit groupInfoFromList(childId + "@g.us", author, subject, creation,
                                               subject_o, subject_t);
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
                                emit leaveGroup(groupId);
                                Utilities::logData("Leaving group: " + groupId);
                            }
                        }
                    }

                    if (child.getTag() == "query")
                    {
                        QString xmlns = child.getAttributeValue("xmlns");

                        if (xmlns == "jabber:iq:last")
                        {
                            qint64 timestamp = QDateTime::currentMSecsSinceEpoch() -
                                    (child.getAttributeValue("seconds").toLongLong() * 1000);

                            emit lastOnline(from, timestamp);
                        }
                    }

                    if (child.getTag() == "media")
                    {
                        Key k(JID_DOMAIN,true,id);
                        FMessage message = store.value(k);

                        if (message.key.id == id)
                        {

                            message.status = FMessage::ReceivedByTarget;
                            message.media_url = child.getAttributeValue("url");
                            store.remove(k);

                            emit mediaUploadAccepted(message);
                        }
                    }
                }
            }

        }

        if (tag == "presence")
        {
            QString xmlns = node.getAttributeValue("xmlns");
            QString from = node.getAttributeValue("from");
            if ((xmlns.isEmpty() || xmlns == "urn:xmpp") && !from.isEmpty() && from != myJid)
            {
                QString type = node.getAttributeValue("type");
                if (type.isEmpty() || type == "available")
                    emit available(from, true);
                else if (type == "unavailable")
                    emit available(from, false);
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

        if (tag == "message")
            parseMessageInitialTagAlreadyChecked(node);

        return true;
    }

    return false;
}

void Connection::parseMessageInitialTagAlreadyChecked(ProtocolTreeNode& messageNode)
{
    ChatMessageType msgType = Unknown;

    QString id = messageNode.getAttributeValue("id");
    QString attribute_t = messageNode.getAttributeValue("t");
    QString from = messageNode.getAttributeValue("from");
    QString author = messageNode.getAttributeValue("author");
    QString typeAttribute = messageNode.getAttributeValue("type");

    if (typeAttribute == "chat")
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

                msgType = MessageReceived;
                sendMessageReceived(message);

            }

            if (child.getTag() == "media")
            {
                // New mms received

                Key k(from, false, id);
                message.setKey(k);
                message.remote_resource = author;
                message.type = FMessage::MediaMessage;

                message.setMediaWAType(child.getAttributeValue("type"));
                message.media_url = child.getAttributeValue("url");
                message.media_name = child.getAttributeValue("file");
                message.media_size = child.getAttributeValue("size").toLongLong();

                QString encoding = child.getAttributeValue("encoding");
                if (encoding == "text" || encoding == "raw")
                    message.setData(child.getData());

                msgType = MessageReceived;
                sendMessageReceived(message);
            }

            if (child.getTag() == "notify")
            {
                QString xmlns = child.getAttributeValue("xmlns");
                if (xmlns == "urn:xmpp:whatsapp")
                {
                    QString notify_name = child.getAttributeValue("name");
                    message.notify_name = notify_name;
                }
            }

            if (child.getTag() == "x")
            {
                QString xmlns = child.getAttributeValue("xmlns");
                if (xmlns == "jabber:x:event" && !id.isEmpty())
                {
                    Key k(from, true, id);
                    message = store.value(k);
                    message.status = FMessage::ReceivedByServer;

                    msgType = (from == "s.us") ? UserStatusUpdate : MessageStatusUpdate;
                }
                else if (xmlns == "jabber:x:delay")
                {
                    QString stamp = child.getAttributeValue("stamp");
                    message.timestamp = DateTimeUtilities::stampParser(stamp);
                }
            }

            if (child.getTag() == "received")
            {
                QString receipt_type = child.getAttributeValue("type");
                Key k(from,true,id);
                message = store.value(k);
                if (message.key.id == id)
                {
                    message.status = FMessage::ReceivedByTarget;
                    msgType = (from == "s.us") ? Unknown : MessageStatusUpdate;
                    store.remove(k);
                }
                if (receipt_type == "delivered" || receipt_type.isEmpty())
                {
                    // Delivery Receipt received
                    sendDeliveredReceiptAck(from,id);
                }
            }

            if (child.getTag() == "composing")
            {
                QString xmlns = child.getAttributeValue("xmlns");
                if (xmlns == "http://jabber.org/protocol/chatstates")
                    msgType = Composing;
            }

            if (child.getTag() == "paused")
            {
                QString xmlns = child.getAttributeValue("xmlns");
                if (xmlns == "http://jabber.org/protocol/chatstates")
                    msgType = Paused;
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

            case Composing:
                emit composing(from);
                break;

            case Paused:
                emit paused(from);
                break;

            case UserStatusUpdate:
                emit userStatusUpdated(message.data);
                break;

            default:
                break;
        }
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
    else if (typeAttribute == "notification")
    {
        ProtocolTreeNode notificationNode = messageNode.getChild("notification");
        QString notificationType = notificationNode.getAttributeValue("type");

        bool notificationReceived = false;

        if (notificationType == "picture")
        {
            ProtocolTreeNodeListIterator i(notificationNode.getChildren());

            while (i.hasNext())
            {
                ProtocolTreeNode child = i.next().value();

                if (child.getTag() == "set")
                {
                    QString photoId = child.getAttributeValue("id");
                    if (!id.isEmpty())
                    {
                        QString author = child.getAttributeValue("author");

                        // emit photoChanged(author, photoId);

                        notificationReceived = true;
                    }
                }

                if (child.getTag() == "delete")
                {
                    notificationReceived = true;
                }

            }
        }

        if (notificationReceived)
            sendNotificationReceived(from, id);
    }

}


void Connection::sendMessageReceived(FMessage& message)
{
    AttributeList attrs;

    ProtocolTreeNode receivedNode("received");
    attrs.insert("xmlns","urn:xmpp:receipts");
    receivedNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("message");
    attrs.clear();
    attrs.insert("to",message.key.remote_jid);
    attrs.insert("type","chat");
    attrs.insert("id",message.key.id);
    messageNode.setAttributes(attrs);
    messageNode.addChild(receivedNode);

    out->write(messageNode);
}

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

    out->write(messageNode);
}


void Connection::sendNotificationReceived(QString to, QString id)
{
    AttributeList attrs;

    ProtocolTreeNode receivedNode("received");
    attrs.insert("xmlns","urn:xmpp:receipts");
    receivedNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("message");
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("type","notification");
    attrs.insert("id",id);
    messageNode.setAttributes(attrs);
    messageNode.addChild(receivedNode);

    out->write(messageNode);
}

void Connection::getReceiptAck(ProtocolTreeNode& node, QString to, QString id,
                               QString receiptType)
{
    ProtocolTreeNode ackNode("ack");
    AttributeList attrs;
    attrs.insert("xmlns","urn:xmpp:receipts");
    attrs.insert("type",receiptType);
    ackNode.setAttributes(attrs);

    node.setTag("message");
    node.addChild(ackNode);
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("type","chat");
    attrs.insert("id",id);
    node.setAttributes(attrs);
}

void Connection::sendDeliveredReceiptAck(QString to, QString id)
{
    ProtocolTreeNode node;
    getReceiptAck(node,to,id,"delivered");

    out->write(node);
}

void Connection::sendNop()
{
    ProtocolTreeNode empty;

    out->write(empty);
}

void Connection::sendPong(QString id)
{
    AttributeList attrs;
    ProtocolTreeNode iqNode("iq");

    attrs.insert("id",id);
    attrs.insert("type","result");
    attrs.insert("to",domain);
    iqNode.setAttributes(attrs);

    out->write(iqNode);
}


QString Connection::makeId()
{
    return QString::number(++iqid,16);
}

qint64 Connection::getLastTreeReadTimestamp()
{
    return lastTreeRead;
}

void Connection::sendComposing(QString to)
{
    AttributeList attrs;

    ProtocolTreeNode composingNode("composing");
    attrs.insert("xmlns","http://jabber.org/protocol/chatstates");
    composingNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("message");
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("type","chat");
    messageNode.setAttributes(attrs);
    messageNode.addChild(composingNode);

    out->write(messageNode);
}

void Connection::sendPaused(QString to)
{
    AttributeList attrs;

    ProtocolTreeNode pausedNode("paused");
    attrs.insert("xmlns","http://jabber.org/protocol/chatstates");
    pausedNode.setAttributes(attrs);

    ProtocolTreeNode messageNode("message");
    attrs.clear();
    attrs.insert("to",to);
    attrs.insert("type","chat");
    messageNode.setAttributes(attrs);
    messageNode.addChild(pausedNode);

    out->write(messageNode);
}

void Connection::updateGroupChats()
{
    QString id = makeId();
    sendGetGroups(id,"participating");
}

void Connection::sendGetGroups(QString id, QString type)
{
    AttributeList attrs;

    ProtocolTreeNode listNode("list");
    attrs.insert("xmlns","w:g");
    attrs.insert("type",type);
    listNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","get");
    attrs.insert("to","g.us");
    iqNode.setAttributes(attrs);
    iqNode.addChild(listNode);

    out->write(iqNode);
}

void Connection::sendSetGroupSubject(QString gjid, QString subject)
{
    QString id = makeId();

    AttributeList attrs;

    ProtocolTreeNode subjectNode("subject");
    attrs.insert("xmlns","w:g");
    attrs.insert("value",subject);
    subjectNode.setAttributes(attrs);

    ProtocolTreeNode iqNode("iq");

    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to",gjid);
    iqNode.setAttributes(attrs);
    iqNode.addChild(subjectNode);

    out->write(iqNode);
}

void Connection::sendLeaveGroup(QString gjid)
{
    QString id = makeId();

    AttributeList attrs;

    ProtocolTreeNode groupNode("group");
    attrs.insert("id",gjid);
    groupNode.setAttributes(attrs);

    ProtocolTreeNode leaveNode("leave");
    attrs.clear();
    attrs.insert("xmlns","w:g");
    leaveNode.setAttributes(attrs);
    leaveNode.addChild(groupNode);

    ProtocolTreeNode iqNode("iq");
    attrs.clear();
    attrs.insert("id",id);
    attrs.insert("type","set");
    attrs.insert("to","g.us");
    iqNode.setAttributes(attrs);
    iqNode.addChild(leaveNode);

    out->write(iqNode);
}

void Connection::setNewUserName(QString push_name)
{
    this->push_name = push_name;
    sendAvailableForChat();
}

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

    QString id = makeId();

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

    out->write(iqNode);
}


void Connection::sendQueryLastOnline(QString jid)
{
    AttributeList attrs;

    QString id = makeId();

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

    out->write(iqNode);
}


