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

#ifndef CHATAREA_H
#define CHATAREA_H

#include <QScrollBar>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QHash>
#include <QEvent>

#include "Whatsapp/fmessage.h"

class ChatArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit ChatArea(QWidget *parent = 0);
    void init();
    void loadLogMessages(QList<FMessage> messages);
    void goToBottom();
    void requestUpdateTimestamps();

signals:
    void topReached();
    void updateTimestamps();
    void mediaDownload(FMessage message);
    void voiceNotePlayed(FMessage message);
    void contextMenuRequested(QPoint p, QObject *obj);

public slots:
    void insertMessageAtTop(FMessage message);
    void insertMessageAtBottom(FMessage message);
    void mediaDownloadHandler(FMessage message);
    void voiceNotePlayedHandler(FMessage message);
    void updateStatus(FMessage message);
    void updateProgress(FMessage message, float p);
    void updateUri(FMessage message);
    void updateImage(FMessage message);
    void resetButton(FMessage message);
    void deleteAllMessages();

private slots:
    void sliderRangeChanged(int min,int max);
    void sliderMoved(int value);
    void showPrevWidget();
    void contextMenu(QPoint p, QObject *obj);

private:
    QVBoxLayout *grid;
    QWidget* container;
    QWidget* widgetAtBottom;
    QHash<Key,QWidget *> widgets;
    QList<QWidget *> widgetList;
    bool loadingMessages;

    void insertMessage(FMessage message, bool loadingMessages);
    void insertBodyMessage(FMessage message,bool loadingMessages);
    void insertMediaMessage(FMessage message,bool loadingMessages);
    QString processMessage(QString data);

protected:
    //bool event(QEvent *);
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // CHATAREA_H
