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

#include "smslistener.h"

#include "util/utilities.h"

SMSListener::SMSListener(QObject *parent) :
    QObject(parent)
{
    // Manager for listening messages
    m_manager = new QMessageManager(this);

    // Listen new added messages
    connect(m_manager,SIGNAL(messageAdded(const QMessageId &,
                                          const QMessageManager::NotificationFilterIdSet &)),
            this, SLOT(messageAdded(const QMessageId&,
                                    const QMessageManager::NotificationFilterIdSet&)));

    // Create 2 filers set for filtering messages
    // - SMS filter
    // - InboxFolder filter
    m_notifFilterSet.insert(m_manager->registerNotificationFilter(
        QMessageFilter::byType(QMessage::Sms) &
        QMessageFilter::byStandardFolder(QMessage::InboxFolder)));
}

void SMSListener::messageAdded(const QMessageId& id,
                               const QMessageManager::NotificationFilterIdSet& matchingFilterIds)
{
    // New message received
    if (matchingFilterIds.contains(m_notifFilterSet)) {
        // New SMS in the inbox
        m_messageId = id;

        QMessage message = m_manager->message(m_messageId);
        // SMS message body
        QString messageString = message.textContent();

        if (messageString.left(14) == "WhatsApp code ")
        {
            // Extract the 7 characters from the right
            // Example SMS message is:
            // WhatsApp code 598-334
            QString code = messageString.right(7);
            code.replace('-',"");
            Utilities::logData("Received WhatsApp code: " + code);

            // Remove message from inbox
            m_manager->removeMessage(m_messageId);

            emit codeReceived(this,code);
        }
    }
}
