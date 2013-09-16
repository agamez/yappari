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

#include "audioplayer.h"

#include "Whatsapp/util/utilities.h"

/**
    Constructs an AudioPlayer object.

    @param parent       QObject parent.
*/
AudioPlayer::AudioPlayer(QObject *parent) :
    QObject(parent)
{
}

/**
    Callback function for the GStreamer asynchronous bus message subsystem.

    Used to detect errors and the end of stream (EOS).

    @param bus          GStreamer bus
    @param msg          GStreamer Message
    @param user_data    Points to this AudioPlayer object
*/
static gboolean bus_call(GstBus *bus, GstMessage *msg, void *user_data)
{
    Q_UNUSED(bus);

    AudioPlayer *obj = (AudioPlayer *) user_data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS: {
                QTimer::singleShot(100,obj,SLOT(freeResources()));
                break;
        }
        case GST_MESSAGE_ERROR: {
                GError *err;
                gst_message_parse_error(msg, &err, NULL);
                Utilities::logData("GStream error: " + QString(err->message));
                g_error_free(err);

                QTimer::singleShot(100,obj,SLOT(freeResources()));
                break;
        }
        default:
                break;
    }

    return true;
}

/**
    Plays a local file through the PulseAudio server using GSTreamer.

    @param uri          Local file URI.  It should start with file://
*/
void AudioPlayer::play(QString uri)
{
    // We need to define the PulseAudio property event.id as ringtone-preview
    // to play the file even when the Silent profile is selected.

    proplist = pa_proplist_new();
    pa_proplist_sets (proplist, PA_PROP_EVENT_ID, "ringtone-preview");

    pulsesink = gst_element_factory_make("pulsesink", NULL);

    g_object_set(G_OBJECT(pulsesink), "proplist", proplist, NULL);

    pipeline = gst_element_factory_make("playbin2", NULL);

    g_object_set(G_OBJECT(pipeline), "uri", qPrintable(uri), NULL);
    g_object_set(G_OBJECT(pipeline), "audio-sink", pulsesink, NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, bus_call, this);

    // Starts the timer that queries current position in the stream
    // to get signals every second of the stream progress
    startPosTimer();

    // Send the "play" signal to the pipeline
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}


/**
    Frees all resources used by this object.

    Also this function indicates that the playing has been finished.

*/
void AudioPlayer::freeResources()
{
    stopPosTimer();

    emit finished();

    gst_object_unref(bus);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(pipeline));

    gst_object_unref(GST_OBJECT(pulsesink));

    // The next line always causes a segmentation fault
    // pa_proplist_free(proplist);

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
void AudioPlayer::startPosTimer()
{
    posTimer = new QTimer(this);
    connect(posTimer,SIGNAL(timeout()),this,SLOT(queryPosition()));

    posTimer->start(1000);
}

/**
    Stops the current position timer.

*/
void AudioPlayer::stopPosTimer()
{
    posTimer->stop();
    posTimer->deleteLater();
}

/**
    Query the current position in the stream and sends the progress signal

*/
void AudioPlayer::queryPosition()
{
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 current = -1;

    if (gst_element_query_position(pipeline, &fmt, &current))
        emit progress((int)(current / 1000000000));
}
