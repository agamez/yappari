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

#ifndef GLOBALCONSTANTS_H
#define GLOBALCONSTANTS_H

// Application constants

#define YAPPARI_APPLICATION_NAME    "Yappari"

// Icons

#define YAPPARI_ICON                "yappari"
#define YAPPARI_NOTIFICATION_ICON   "yappari-notification"

// Whatsapp client spoofing

#define USER_AGENT_VERSION  "2.9.6621"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" Android/2.3.4 Device/samsung-GT-I9100"
#define RESOURCE	    "Android-"USER_AGENT_VERSION"-443"

/*
#define USER_AGENT_VERSION  "2.4.22"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" S40Version/14.26 Device/Nokia302"
#define RESOURCE	    "S40-"USER_AGENT_VERSION"-443"
*/

/*
#define USER_AGENT_VERSION  "2.8.3"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" iPhone_OS/5.0.1 Device/Unknown_(iPhone4,1)"
#define RESOURCE            "iPhone-"USER_AGENT_VERSION


#define USER_AGENT_VERSION  "2.8.2"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" WP7/7.50 Device/Nokia-Lumia_900-1.0"
#define RESOURCE            "WP7-"USER_AGENT_VERSION"-5222"
*/

// WhatsApp URLs
#define URL_REGISTRATION_V2     "https://v.whatsapp.net/v2/"
#define URL_CONTACTS_AUTH       "https://sro.whatsapp.net/v2/sync/a"
#define URL_CONTACTS_SYNC       "https://sro.whatsapp.net/v2/sync/q"

// WhatsApp Servers
#define JID_DOMAIN              "s.whatsapp.net"
#define SERVER_DOMAIN           "c.whatsapp.net"

// DBus Services

#define YAPPARI_SERVICE                         "org.scorpius.Yappari"
#define YAPPARI_OBJECT                          "/org/scorpius/Yappari"

#define APPLET_SERVICE                          "org.scorpius.YappariApplet"
#define APPLET_OBJECT                           "/org/scorpius/YappariApplet"

#define NOKIA_PROFILE_SERVICE                   "com.nokia.profiled"
#define NOKIA_PROFILE_OBJECT                    "/com/nokia/profiled"
#define NOKIA_PROFILE_VIBRATING_ALERT_ENABLED   "vibrating.alert.enabled"
#define NOKIA_PROFILE_IM_ALERT_TONE             "im.alert.tone"

#define NOKIA_MCE_SERVICE                       "com.nokia.mce"
#define NOKIA_MCE_OBJECT                        "/com/nokia/mce/request"
#define NOKIA_MCE_VIBRATE_PATTERN               "PatternChatAndEmail"

#define NOKIA_MEDIAPLAYER_DBUS_NAME             "com.nokia.mediaplayer"
#define NOKIA_MEDIAPLAYER_DBUS_PATH             "/com/nokia/mediaplayer"

#define NOKIA_IMAGEVIEWER_DBUS_NAME             "com.nokia.image_viewer"
#define NOKIA_IMAGEVIEWER_DBUS_PATH             "/com/nokia/image_viewer"

#define HD_NOTIFICATION_MANAGER_DBUS_NAME       "org.freedesktop.Notifications"
#define HD_NOTIFICATION_MANAGER_DBUS_PATH       "/org/freedesktop/Notifications"

#define HD_SV_NOTIFICATION_DAEMON_DBUS_NAME     "com.nokia.HildonSVNotificationDaemon"
#define HD_SV_NOTIFICATION_DAEMON_DBUS_PATH     "/com/nokia/HildonSVNotificationDaemon"

// Settings

#define SETTINGS_ORGANIZATION               "scorpius"
#define SETTINGS_APPLICATION                "yappari"

#define SETTINGS_USERNAME                   "username"
#define SETTINGS_PHONENUMBER                "phonenumber"
#define SETTINGS_PASSWORD                   "password"
#define SETTINGS_CC                         "cc"
#define SETTINGS_NUMBER                     "number"
#define SETTINGS_IMEI                       "imei"
#define SETTINGS_REGISTERED                 "registered"
#define SETTINGS_NICKCOLOR                  "nickcolor"
#define SETTINGS_MYCOLOR                    "mycolor"
#define SETTINGS_TEXTCOLOR                  "textcolor"
#define SETTINGS_STATUS                     "status"
#define SETTINGS_SYNC                       "sync"
#define SETTINGS_ANDROID                    "android"
#define SETTINGS_NEXTCHALLENGE              "nextchallenge"
#define SETTINGS_CREATION                   "creation"
#define SETTINGS_EXPIRATION                 "expiration"
#define SETTINGS_ACCOUNTSTATUS              "accountstatus"
#define SETTINGS_KIND                       "kind"
#define SETTINGS_PORT                       "port"
#define SETTINGS_SHOW_NICKNAMES             "shownicknames"
#define SETTINGS_SHOW_NUMBERS               "shownumbers"
#define SETTINGS_POPUP_ON_FIRST_MESSAGE     "popuponfirstmessage"
#define SETTINGS_AUTOMATIC_DOWNLOAD         "automaticdownload"
#define SETTINGS_IMPORT_TO_GALLERY          "importmediatogallery"

#define SETTINGS_WHATSNEW                   "whatsnew"

// Default settings values

#define DEFAULT_PORT                        443
#define DEFAULT_SHOW_NICKNAMES              false
#define DEFAULT_SHOW_NUMBERS                true
#define DEFAULT_POPUP_ON_FIRST_MESSAGE      false
#define DEFAULT_AUTOMATIC_DOWNLOAD          100
#define DEFAULT_IMPORT_TO_GALLERY           false

// Synchronization options

#define SYNC_ENABLED        "on"
#define SYNC_INTL_ONLY      "intl"
#define SYNC_DISABLED       "off"


// Status

#define DEFAULT_STATUS          "I am using Yappari!"

// Username

#define DEFAULT_USERNAME        "Yappari User"

// Timers

#define MIN_INTERVAL            300000
#define RETRY_LOGIN_INTERVAL     10000
#define CHECK_QUEUE_INTERVAL      1000

// Directories

#define HOME_DIR        "/.yappari/"
#define YAPPARI_DIR     "/Yappari"
#define LOGS_DIR        HOME_DIR"logs/"
#define CONF_DIR        "/.config/scorpius"
#define DEFAULT_DIR     "/MyDocs"
#define CACHE_DIR       "/.yappari/cache"
#define IMAGES_DIR      "images"
#define VIDEOS_DIR      "videos"
#define AUDIO_DIR       "sounds"

#define GRAYCHECK       "<img src=\"/usr/share/yappari/icons/17x13/yappari-graycheck.png\" />"
#define CHECK           "<img src=\"/usr/share/yappari/icons/17x13/yappari-greencheck.png\" />"
#define DOUBLECHECK     "<img src=\"/usr/share/yappari/icons/17x13/yappari-greendoublecheck.png\" />"
#define UPLOADING       "<img src=\"/usr/share/yappari/icons/17x13/yappari-uploading.png\" />"

// Extensions

#define EXTENSIONS_VIDEO   "Videos (*.avi *.mpg *.mpeg *.mpe *.3gp *.wmv *.mp4 *.mov *.qt *.AVI *.MPG *.MPEG *.MPE *.3GP *.WMV *.MP4 *.MOV *.QT)"
#define EXTENSIONS_IMAGE   "Images (*.png *.jpg *.jpeg *.gif *.PNG *.JPG *.JPEG *.GIF)"
#define EXTENSIONS_AUDIO   "Audio Files (*.aac *.mp3 *.m4a *.wma *.wav *.ogg *.AAC *.MP3 *.M4A *.WMA *.WAV *.OGG)"

// Donate URL

#define DONATE_URL      "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=NVDVRMP4WTFJU&lc=US&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHosted"
#define MAGIC_NUMBER    529031862

#endif // GLOBALCONSTANTS_H
