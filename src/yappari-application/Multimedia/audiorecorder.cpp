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

#include <QDateTime>
#include <QDir>

#include "audiorecorder.h"

#include "Whatsapp/util/utilities.h"

#include "globalconstants.h"


/** ***********************************************************************
 ** Local definitions
 **/

// Prefix for the resulting files of the recordings
#define PUSH_TO_TALK_FILENAME_PREFIX        "voice_"

/**
    Constructs an AudioRecorder object.

    @param parent       QObject parent.
*/
AudioRecorder::AudioRecorder(QString codec, QObject *parent) :
    QObject(parent)
{
    this->codec = codec;

    // Check the directories are created
    QDir home = QDir::home();
    QString folder = home.path() + CACHE_DIR "/" AUDIO_DIR;
    if (!home.exists(folder))
        home.mkpath(folder);
}

/**
    Starts the recording of audio

    This function uses GStream to record the audio.

    The equivalent command line is:
    gst-launch autoaudiosrc ! audioconvert ! nokiaaacenc output-format=1 ! filesink location=filename.aac

*/
void AudioRecorder::record()
{
    // Generate the file name

    QDir home = QDir::home();
    fileName = home.path() + CACHE_DIR "/" AUDIO_DIR "/" PUSH_TO_TALK_FILENAME_PREFIX +
            QString::number(QDateTime::currentMSecsSinceEpoch()) + "." + codec;


#ifndef Q_WS_SCRATCHBOX

    // Create the pipeline
    // This pipeline is equivalent to the following command:
    // gst-launch autoaudiosrc ! audioconvert ! nokiaaacenc output-format=1 ! filesink location=filename.aac

    recordBin = gst_pipeline_new("record-pipeline");
    g_assert(recordBin);

    audioSrc = gst_element_factory_make("autoaudiosrc", NULL);
    g_assert(audioSrc);

    audioConvert = gst_element_factory_make("audioconvert", NULL);
    g_assert(audioConvert);

    fileSink = gst_element_factory_make("filesink", NULL);
    g_assert(fileSink);

    if (codec == AAC)
    {
        voiceEncoder = gst_element_factory_make("nokiaaacenc", NULL);
        g_object_set(G_OBJECT(voiceEncoder), "output-format", 1, NULL);
        g_assert(voiceEncoder);
    }
    else
    {
        voiceEncoder = gst_element_factory_make("nokiaamrnbenc", NULL);
        g_object_set(G_OBJECT(fileSink), "append", 1, NULL);
        g_assert(voiceEncoder);

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            emit finished(fileName,0);

        QTextStream out(&file);
        out << "#!AMR\n";

        file.close();
    }

    gst_bin_add_many(GST_BIN(recordBin), audioSrc, audioConvert, voiceEncoder, fileSink, NULL);
    gst_element_link_many(audioSrc, audioConvert, voiceEncoder, fileSink, NULL);

    // Start recording

    gst_element_set_state(GST_ELEMENT(recordBin), GST_STATE_NULL);
    g_object_set(G_OBJECT(fileSink), "location", qPrintable(fileName), NULL);

    isRecording = true;
    gst_element_set_state(GST_ELEMENT(recordBin), GST_STATE_PLAYING);

#endif

    // Start the timer that queries the position (in seconds) in the stream
    startPosTimer();

    Utilities::logData("AudioRecord: recording...");
}


/**
    Prepares to stop the recording.

    What this method actually does is to record for 0.5 more seconds to avoid weird cuts
    in the voice notes.

*/
void AudioRecorder::stop()
{
    int delay = (codec == AAC) ? 500 : 1000;
    QTimer::singleShot(delay,this,SLOT(stopRecording()));
}

/**
    Stops the recording.

    This method stops the recording and send the signal finished with the resulting fileName
    and the length in seconds of the stream.

    @param parent       QObject parent.
*/
void AudioRecorder::stopRecording()
{
    recordingMutex.lock();
    if (isRecording)
    {
        isRecording = false;
        stopPosTimer();

#ifndef Q_WS_SCRATCHBOX
        GstFormat fmt = GST_FORMAT_TIME;
        gint64 current = -1;

        gst_element_query_position(recordBin, &fmt, &current);
        gst_element_set_state(GST_ELEMENT(recordBin), GST_STATE_READY);

        emit finished(fileName, ((int)(current / 1000000000)));
#else
        emit finished(fileName, 0);
#endif

        Utilities::logData("AudioRecord: stopped. Filename: " + fileName);
    }

    recordingMutex.unlock();

#ifndef Q_WS_SCRATCHBOX
    gst_object_unref(GST_OBJECT(audioSrc));
    gst_object_unref(GST_OBJECT(audioConvert));
    gst_object_unref(GST_OBJECT(voiceEncoder));
    gst_object_unref(GST_OBJECT(fileSink));
    gst_object_unref(GST_OBJECT(recordBin));
#endif

    deleteLater();
}


/** ***********************************************************************
 ** Current stream position timer methods
 **/

/**
    Starts the current position timer.

    This timer will wake every second to emit stream current position
    updates.

*/
void AudioRecorder::startPosTimer()
{
    posTimer = new QTimer(this);
    connect(posTimer,SIGNAL(timeout()),this,SLOT(queryPosition()));

    posTimer->start(1000);
}

/**
    Stops the current position timer.

*/
void AudioRecorder::stopPosTimer()
{
    posTimer->stop();
    posTimer->deleteLater();
}

/**
    Query the current position in the stream and sends the progress signal

*/
void AudioRecorder::queryPosition()
{
#ifndef Q_WS_SCRATCHBOX
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current = -1;

    if (gst_element_query_position(recordBin, &fmt, &current))
        emit progress((int)(current / 1000000000));
#else
    emit progress(0);
#endif
}
