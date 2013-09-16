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

#ifndef AUDIORECORDER_H
#define AUDIORECORDER_H

#include <QObject>
#include <QMutex>
#include <QTimer>

#include <gst/gst.h>
#include <stdbool.h>
#include <pulse/proplist.h>

/**
    @class      AudioRecorder

    @brief      This class provides a simple audio recorder using the GStreamer API.

                It includes notifications every second to indicate the current
                position in the stream (in seconds).

                After the recording is done this object is deleted.
*/

class AudioRecorder : public QObject
{
    Q_OBJECT

public:

    /** ***********************************************************************
     ** Public members
     **/


    /** ***********************************************************************
     ** Constructors and destructors
     **/

    // Creates an AudioRecorder object
    explicit AudioRecorder(QString codec, QObject *parent = 0);


public slots:

    /** ***********************************************************************
     ** Public slots methods
     **/

    // Starts recording a stream
    void record();

    // Signals stop recording to avoid stream cuts
    void stop();

    // Stops recording
    void stopRecording();

    // Queries the current time in seconds of the stream
    void queryPosition();


signals:

    /** ***********************************************************************
     ** Signals
     **/

    // Progress in seconds in the stream being recorderd
    void progress(int current);

    // Stream recorded successfully
    void finished(QString fileName, int lengthInSeconds);
    


private:

    /** ***********************************************************************
     ** Private members
     **/

    // Codec
    QString codec;

    // GStream record pipeline
    GstElement *recordBin;

    // GStream plugin for automatically detected audio source
    GstElement *audioSrc;

    // GStream plugin for audio format conversion
    GstElement *audioConvert;

    // GStream Nokia plugin to encode AAC audio
    GstElement *voiceEncoder;

    // GStream plugin to save audio to a file
    GstElement *fileSink;

    // File name where the resulting audio is going to be stored
    QString fileName;

    // True if this object is recording audio
    bool isRecording;

    // Mutual exclusion object to avoid double recording
    QMutex recordingMutex;

    // Timer used to query current position in seconds in the stream
    QTimer *posTimer;


    /** ***********************************************************************
     ** Private methods
     **/

    // Starts the timer to query current position in the stream
    void startPosTimer();

    // Stops the timer to query current position in the stream
    void stopPosTimer();
};

#endif // AUDIORECORDER_H
