/* Copyright 2012 Naikel Aparicio. All rights reserved.
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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSystemInfo>
#include <QTcpSocket>
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

class Connection : public QObject
{
    Q_OBJECT

public:
    enum ChatMessageType {
        Unknown,
        MessageReceived,
        MessageStatusUpdate,
        Composing,
        Paused,
        UserStatusUpdate
    };

    QTcpSocket *socket;
    QString domain;
    QString resource;
    QString user;
    QString push_name;
    QByteArray password;
    QString myJid;

    QByteArray nextChallenge;
    QString creation;
    QString expiration;
    QString kind;
    QString accountstatus;

    BinTreeNodeWriter *out;
    BinTreeNodeReader *in;

    explicit Connection(QTcpSocket *socket, QString domain, QString resource,
                        QString user, QString push_name, QByteArray password,
                        DataCounters *counters, QObject *parent = 0);
    ~Connection();

    void login(QByteArray nextChallenge);
    bool read();
    void ping();
    qint64 getLastTreeReadTimestamp();

    void updateGroupChats();
    void setNewUserName(QString push_name);

private:
    static FunStore store;
    KeyStream *outputKey;
    KeyStream *inputKey;
    QStringList dictionary;
    qint64 lastTreeRead;
    int iqid;
    DataCounters *counters;

    ProtocolTreeNode getMessageNode(FMessage& message, ProtocolTreeNode& child);
    int sendFeatures();
    int sendAuth();
    QByteArray getAuthBlob(QByteArray nonce);
    QByteArray readFeaturesUntilChallengeOrSuccess(int *bytes);
    int sendResponse(QByteArray challengeData);
    void parseSuccessNode(ProtocolTreeNode& node);
    int readSuccess();
    void sendClientConfig(QString platform);
    void sendAvailableForChat();

    void parseMessageInitialTagAlreadyChecked(ProtocolTreeNode& messageNode);
    void sendMessageReceived(FMessage& message);
    void sendSubjectReceived(QString to, QString id);
    void sendNotificationReceived(QString to, QString id);
    void getReceiptAck(ProtocolTreeNode& node, QString to, QString id, QString receiptType);
    void sendDeliveredReceiptAck(QString to, QString id);
    QString makeId(QString prefix);
    void sendMessageWithBody(FMessage& message);
    void sendMessageWithMedia(FMessage& message);
    void requestMessageWithMedia(FMessage& message);
    void sendComposing(QString to);
    void sendPaused(QString to);
    void sendPong(QString id);
    void sendGetGroups(QString id,QString type);


signals:
    void connectionClosed();
    void messageReceived(FMessage message);
    void messageStatusUpdate(FMessage message);
    void groupInfoFromList(QString id, QString from, QString author,
                           QString newSubject, QString creation,
                           QString subjectOwner, QString subjectTimestamp);
    void groupNewSubject(QString from, QString author, QString authorName,
                         QString newSubject, QString creation);
    void groupAddUser(QString from, QString remove);
    void groupRemoveUser(QString from, QString remove);
    void available(QString jid,bool online);
    void composing(QString jid);
    void paused(QString jid);
    void leaveGroup(QString gjid);
    void userStatusUpdated(FMessage message);
    void lastOnline(QString jid, qint64 timestamp);
    void mediaUploadAccepted(FMessage message);
    void photoIdReceived(QString jid, QString pictureId);
    void photoDeleted(QString jid);
    void photoReceived(QString from, QByteArray data,
                       QString photoId, bool largeFormat);
    void addParticipant(QString gjid, QString jid);
    void groupParticipant(QString gjid, QString jid);
    void groupError(QString gjid);


public slots:
    void sendMessage(FMessage& message);
    void sendNop();
    void sendSetGroupSubject(QString gjid, QString subject);
    void sendLeaveGroup(QString gjid);
    void sendQueryLastOnline(QString jid);
    void sendGetPhotoIds(QStringList jids);
    void sendGetPhoto(QString jid, QString expectedPhotoId, bool largeFormat);
    void sendGetStatus(QString jid);
    void sendSetPhoto(QString jid, QByteArray imageBytes, QByteArray thumbBytes);
    void sendPresenceSubscriptionRequest(QString jid);
    void sendUnsubscribeHim(QString jid);
    void sendCreateGroupChat(QString subject, QString id);
    void sendAddParticipants(QString gjid, QStringList participants);
    void sendRemoveParticipants(QString gjid, QStringList participants);
    void sendVerbParticipants(QString gjid, QStringList participants,
                              QString id, QString innerTag);
    void sendGetParticipants(QString gjid);

};


#endif // CONNECTION_H
