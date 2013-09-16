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

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QTimer>

#include <gst/gst.h>
#include <stdbool.h>
#include <pulse/proplist.h>

/**
    @class      AudioPlayer

    @brief      This class provides a simple audio player using the GStreamer API.

                It includes notifications every second to indicate the current
                position in the stream (in seconds).

                After the file is played this object is deleted.
*/

class AudioPlayer : public QObject
{
    Q_OBJECT

public:

    /** ***********************************************************************
     ** Public members
     **/


    /** ***********************************************************************
     ** Constructors and destructors
     **/

    // Creates an AudioPlayer object
    explicit AudioPlayer(QObject *parent = 0);

public slots:

    /** ***********************************************************************
     ** Public slots methods
     **/

    // Plays a local file
    void play(QString uri);

    // Queries the current position in the stream played
    void queryPosition();

    // Free all resources used by this object
    void freeResources();

signals:

    /** ***********************************************************************
     ** Signals
     **/

    // Progress in seconds in the stream played
    void progress(int current);

    // Stream played completely
    void finished();
    

private:

    /** ***********************************************************************
     ** Private members
     **/

    // GStreamer pipeline used to play the stream
    GstElement *pipeline;

    // GStreamer plugin to send audio to a PulseAudio server
    GstElement *pulsesink;

    // GStreamer asynchronous message bus subsystem
    GstBus *bus;

    // PulseAudio property list
    pa_proplist *proplist;

    // Timer used to query current position in the stream
    QTimer *posTimer;


    /** ***********************************************************************
     ** Private methods
     **/

    // Starts the timer to query current position in the stream
    void startPosTimer();

    // Stops the timer to query current position in the stream
    void stopPosTimer();
};

#endif // AUDIOPLAYER_H
