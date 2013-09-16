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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSystemInfo>
#include <QTcpSocket>
#include <QTimer>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QMap>
#include <QHash>

#include "util/messagedigest.h"
#include "util/datacounters.h"

#include "protocoltreenode.h"
#include "bintreenodewriter.h"
#include "bintreenodereader.h"
#include "protocolexception.h"
#include "loginexception.h"
#include "keystream.h"
#include "fmessage.h"
#include "funstore.h"

// QtMobility namespace
QTM_USE_NAMESPACE

/**
    @class      Connection

    @brief      The Connection class provides the WhatsApp functionality.
                It represents a connection to the WhatsApp servers and provides
                functions to send and receive WhatsApp messages
*/

class Connection : public QObject
{
    Q_OBJECT

public:

    /** ***********************************************************************
     ** Enumerations
     **/

    // Message Status types
    enum ChatMessageType {
        Unknown,
        MessageReceived,
        MessageStatusUpdate,
        Composing,
        Paused,
        UserStatusUpdate
    };

    // Photo notification type
    enum PhotoOperationType {
        UnknownPhotoOperation,
        SetPhoto,
        DeletePhoto
    };


    /** ***********************************************************************
     ** Public members
     **/

    // Next authentication challenge
    QByteArray nextChallenge;

    // Account creation date (unixtime)
    QString creation;

    // Account expiration date (unixtime)
    QString expiration;

    // Account kind (free / paid)
    QString kind;

    // Account status (active / expired)
    QString accountstatus;


    /** ***********************************************************************
     ** Constructors and destructors
     **/

    // Create a Connection object
    explicit Connection(QTcpSocket *socket, QString domain, QString resource,
                        QString user, QString push_name, QByteArray password,
                        DataCounters *counters, QObject *parent = 0);

    // Destroy a Connection object
    ~Connection();


    /** ***********************************************************************
     ** General Public Methods
     **/

    // Login to the WhatsApp servers
    void login(QByteArray nextChallenge);

    // Read next node
    bool read();

    // Get the unixtime of the last node successfully read
    qint64 getLastTreeReadTimestamp();


public slots:

    /** ***********************************************************************
     ** Public slots methods
     **/


    /** ***********************************************************************
     ** Message handling
     **/

    // Sends a FMessage
    void sendMessage(FMessage& message);


    /** ***********************************************************************
     ** User handling
     **/

    // Sends a query to request the time when a user was last seen online.
    void sendQueryLastOnline(QString jid);

    // Sends a query to get the current status of a user
    void sendGetStatus(QString jid);

    // Sends a query to request a subscription to a user
    void sendPresenceSubscriptionRequest(QString jid);

    // Sends a query to request a subscription remove to a user
    void sendUnsubscribeHim(QString jid);


    /** ***********************************************************************
     ** Picture handling
     **/

    // Sends a query to request to get the current profile picture of a user
    void sendGetPhoto(QString jid, QString expectedPhotoId, bool largeFormat);

    // Sends a request to set a photo
    void sendSetPhoto(QString jid, QByteArray imageBytes, QByteArray thumbBytes);

    // Sends a request to get the photo Ids from a list of jids
    void sendGetPhotoIds(QStringList jids);

    /** ***********************************************************************
     ** Voice notes handling
     **/

    // Sends a notification that a voice note was played
    void sendVoiceNotePlayed(FMessage message);

    /** ***********************************************************************
     ** Group handling
     **/

    // Sends a request to create a group
    void sendCreateGroupChat(QString subject, QString id);

    // Sends a request to add participants to a group
    void sendAddParticipants(QString gjid, QStringList participants);

    // Sends a request to remove participants from a group
    void sendRemoveParticipants(QString gjid, QStringList participants);

    // Sends a request with participants to a group
    void sendVerbParticipants(QString gjid, QStringList participants,
                              QString id, QString innerTag);

    // Sends a request to retrieve the participants list of a group
    void sendGetParticipants(QString gjid);

    // Sends a request to retrieve the participants list of all groups
    void updateGroupChats();

    // Sends a request to change/set a group subject
    void sendSetGroupSubject(QString gjid, QString subject);

    // Sends a request to leave a group
    void sendLeaveGroup(QString gjid);


    /** ***********************************************************************
     ** Privacy list handling
     **/

    // Sends a request to get the privacy list
    void sendGetPrivacyList();

    // Sends a request to set the privacy list
    void sendSetPrivacyBlockedList(QStringList jidList);


    /** ***********************************************************************
     ** General slots
     **/

    // Sends a no operation (ping) to the network
    void sendNop();

    // Sends a ping request to th3e network
    void sendPing();

    // Changes the user name or alias
    void setNewUserName(QString push_name);


    // Connection timeout handling
    void connectionTimeout();

private:

    /** ***********************************************************************
     ** Private members
     **/

    // Store with messages waiting for acks
    static FunStore store;

    // TCP Socket with the TCP connection established
    QTcpSocket *socket;

    // XMPP domain.  Usually "s.whatsapp.net"
    QString domain;

    // Client resource identifier.  Example: "S40-2.4.22-443"
    QString resource;

    // Phone number
    QString user;

    // User name or alias
    QString push_name;

    // User password
    QByteArray password;

    // User jid
    QString myJid;

    // XMPP dictionary
    QStringList dictionary;

    // Timestamp of the last successfully node read
    qint64 lastTreeRead;

    // Unique identifier for <iq> nodes
    int iqid;

    // Pointer to the DataCounters where network counters are being kept
    DataCounters *counters;

    // Writer stream to send nodes
    BinTreeNodeWriter *out;

    // Reader stream to receive nodes
    BinTreeNodeReader *in;

    // Writer crypto stream
    KeyStream *outputKey;

    // Reader crypto stream
    KeyStream *inputKey;

    // Connection timeout timer
    QTimer connTimeout;


    /** ***********************************************************************
     ** Private methods
     **/

    // Parse a <message> node
    void parseMessageInitialTagAlreadyChecked(ProtocolTreeNode& messageNode);


    /** ***********************************************************************
     ** Authentication
     **/

    // Sends the features supported by this client to the WhatsApp servers
    int sendFeatures();

    // Sends the authentication request
    int sendAuth();

    // Constructs the authentication data to be sent
    QByteArray getAuthBlob(QByteArray nonce);

    // Reads the features from the node and the challenge data
    QByteArray readFeaturesUntilChallengeOrSuccess(int *bytes);

    // Sends authentication response
    int sendResponse(QByteArray challengeData);

    // Read authentication success node
    int readSuccess();

    // Parses authentication success node
    void parseSuccessNode(ProtocolTreeNode& node);


    /** ***********************************************************************
     ** Message handling
     **/

    // Sends a text message
    void sendMessageWithBody(FMessage& message);

    // Sends a request to send a multimedia message
    void requestMessageWithMedia(FMessage& message);

    // Sends a multimedia message
    void sendMessageWithMedia(FMessage& message);

    // Constructs a message node
    ProtocolTreeNode getMessageNode(FMessage& message, ProtocolTreeNode& child);

    // Send a message received acknowledgement
    void sendMessageReceived(FMessage& message);

    // Send a notification received acknowledgement
    void sendNotificationReceived(QString to, QString id);

    // Constructs a receipt acknowledge node
    void getReceiptAck(ProtocolTreeNode& node, QString to, QString id,
                       QString receiptType);

    // Sends a receipt acknowledging a delivered message notification received
    void sendDeliveredReceiptAck(QString to, QString id, QString type);


    /** ***********************************************************************
     ** Typing status handling
     **/

    // Sends a notification that the user is composing a message
    void sendComposing(FMessage message);

    // Sends a notification that the user has stopped typing a message
    void sendPaused(FMessage message);


    /** ***********************************************************************
     ** Group handling
     **/

    // Sends a request to all groups
    void sendGetGroups(QString id,QString type);

    // Sends a notification that a group subject was changed
    void sendSubjectReceived(QString to, QString id);


    /** ***********************************************************************
     ** General methods
     **/

    // Sends a ping acknowledge (pong) to the network
    void sendPong(QString id);

    // Constructs an id
    QString makeId(QString prefix);

    // Sends the client configuration
    void sendClientConfig(QString platform);

    // Sends notification that this client is available for chat (online)
    void sendAvailableForChat();

signals:

    /** ***********************************************************************
     ** Message handling
     **/

    // Message received
    void messageReceived(FMessage message);

    // Message status update
    void messageStatusUpdate(FMessage message);

    // Upload of Multimedia message accepted
    void mediaUploadAccepted(FMessage message);


    /** ***********************************************************************
     ** Typing status handling
     **/

    // User is typing
    void composing(QString jid, QString media);

    // User stopped typing
    void paused(QString jid);


    /** ***********************************************************************
     ** User handling
     **/

    // User availability
    void available(QString jid, bool online);

    // Last seen timestamp of user
    void lastOnline(QString jid, qint64 timestamp);

    // User status update
    void userStatusUpdated(FMessage message);


    /** ***********************************************************************
     ** Picture handling
     **/

    // User photo updated notification received
    void photoIdReceived(QString jid, QString alias, QString photoId);

    // User photo has been deleted
    void photoDeleted(QString jid, QString alias);

    // User photo has been received
    void photoReceived(QString from, QByteArray data,
                       QString photoId, bool largeFormat);


    /** ***********************************************************************
     ** Group handling
     **/

    // Received a group from the list of groups obtained by updateGroupChats()
    void groupInfoFromList(QString id, QString from, QString author,
                           QString newSubject, QString creation,
                           QString subjectOwner, QString subjectTimestamp);

    // Received a new group subject (and possibly a new group)
    void groupNewSubject(QString from, QString author, QString authorName,
                         QString newSubject, QString creation);

    // User added to a group
    void groupAddUser(QString from, QString remove);

    // User removed from a group
    void groupRemoveUser(QString from, QString remove);

    // Group left
    void groupLeft(QString gjid);

    // User belongs to group
    void groupUser(QString gjid, QString jid);

    // Group error (not in the group anymore or group doesn't exist anymore)
    void groupError(QString gjid);


    /** ***********************************************************************
     ** Privacy list handling
     **/

    // Privacy list received
    void privacyListReceived(QStringList list);

    /** ***********************************************************************
     ** General signals
     **/

    // Connection timeout
    void timeout();
};


#endif // CONNECTION_H
