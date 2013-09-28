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

#ifndef GLOBALCONSTANTS_H
#define GLOBALCONSTANTS_H

// Application constants

#define YAPPARI_APPLICATION_NAME    "Yappari"

// Icons

#define YAPPARI_ICON                "yappari"
#define YAPPARI_NOTIFICATION_ICON   "yappari-notification"

// WhatsApp Client Spoofing

/*
#define USER_AGENT_VERSION  "2.10.773"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" Android/2.3.4 Device/samsung-GT-I9100"
#define RESOURCE            "Android-"USER_AGENT_VERSION"-443"
*/


#define USER_AGENT_VERSION  "2.11.4"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" S40Version/gadCEX3.60 Device/Nokia305"
#define RESOURCE            "S40-"USER_AGENT_VERSION"-443"


/*
#define USER_AGENT_VERSION  "2.8.3"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" iPhone_OS/5.0.1 Device/Unknown_(iPhone4,1)"
#define RESOURCE            "iPhone-"USER_AGENT_VERSION


#define USER_AGENT_VERSION  "2.8.2"
#define USER_AGENT          "WhatsApp/"USER_AGENT_VERSION" WP7/7.50 Device/Nokia-Lumia_900-1.0"
#define RESOURCE            "WP7-"USER_AGENT_VERSION"-5222"
*/

// WhatsApp Registration Key

/*
#define BUILD_KEY   "k7Iy3bWARdNeSL8gYgY6WveX12A1g4uTNXrRzt1H"
#define BUILD_HASH  "889d4f44e479e6c38b4a834c6d8417815f999abe"
*/


#define BUILD_KEY   "PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk"
#define BUILD_HASH  "1380568664606"


/*
#define BUILD_KEY   "30820332308202f0a00302010202044c2536a4300b06072a8648ce3804030500307c310b3009060355040613025553311330110603550408130a43616c69666f726e6961311430120603550407130b53616e746120436c61726131163014060355040a130d576861747341707020496e632e31143012060355040b130b456e67696e656572696e67311430120603550403130b427269616e204163746f6e301e170d3130303632353233303731365a170d3434303231353233303731365a307c310b3009060355040613025553311330110603550408130a43616c69666f726e6961311430120603550407130b53616e746120436c61726131163014060355040a130d576861747341707020496e632e31143012060355040b130b456e67696e656572696e67311430120603550403130b427269616e204163746f6e308201b83082012c06072a8648ce3804013082011f02818100fd7f53811d75122952df4a9c2eece4e7f611b7523cef4400c31e3f80b6512669455d402251fb593d8d58fabfc5f5ba30f6cb9b556cd7813b801d346ff26660b76b9950a5a49f9fe8047b1022c24fbba9d7feb7c61bf83b57e7c6a8a6150f04fb83f6d3c51ec3023554135a169132f675f3ae2b61d72aeff22203199dd14801c70215009760508f15230bccb292b982a2eb840bf0581cf502818100f7e1a085d69b3ddecbbcab5c36b857b97994afbbfa3aea82f9574c0b3d0782675159578ebad4594fe67107108180b449167123e84c281613b7cf09328cc8a6e13c167a8b547c8d28e0a3ae1e2bb3a675916ea37f0bfa213562f1fb627a01243bcca4f1bea8519089a883dfe15ae59f06928b665e807b552564014c3bfecf492a0381850002818100d1198b4b81687bcf246d41a8a725f0a989a51bce326e84c828e1f556648bd71da487054d6de70fff4b49432b6862aa48fc2a93161b2c15a2ff5e671672dfb576e9d12aaff7369b9a99d04fb29d2bbbb2a503ee41b1ff37887064f41fe2805609063500a8e547349282d15981cdb58a08bede51dd7e9867295b3dfb45ffc6b259300b06072a8648ce3804030500032f00302c021400a602a7477acf841077237be090df436582ca2f0214350ce0268d07e71e55774ab4eacd4d071cd1efad"
#define BUILD_HASH  "a9ef42fa03c92030c1ba432f0db97be6"
*/

// WhatsApp URLs
#define URL_REGISTRATION_V2     "https://v.whatsapp.net/v2/"
#define URL_CONTACTS_AUTH       "https://sro.whatsapp.net/v2/sync/a"
#define URL_CONTACTS_SYNC       "https://sro.whatsapp.net/v2/sync/q"
#define URL_LOCATION_SHARING    "https://maps.google.com/maps?q=loc:"

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

#define NOKIA_CSD_CALL_SERVICE                  "com.nokia.csd.Call"
#define NOKIA_CSD_CALL_OBJECT                   "/com/nokia/csd/call"

#define HD_NOTIFICATION_MANAGER_DBUS_NAME       "org.freedesktop.Notifications"
#define HD_NOTIFICATION_MANAGER_DBUS_PATH       "/org/freedesktop/Notifications"

#define HD_SV_NOTIFICATION_DAEMON_DBUS_NAME     "com.nokia.HildonSVNotificationDaemon"
#define HD_SV_NOTIFICATION_DAEMON_DBUS_PATH     "/com/nokia/HildonSVNotificationDaemon"

// Settings

#define SETTINGS_ORGANIZATION               "scorpius"
#define SETTINGS_APPLICATION                "yappari"
#define SETTINGS_COUNTERS                   "counters"

#define SETTINGS_USERNAME                   "username"
#define SETTINGS_PHONENUMBER                "phonenumber"
#define SETTINGS_PASSWORD                   "password"
#define SETTINGS_CC                         "cc"
#define SETTINGS_NUMBER                     "number"
#define SETTINGS_IMEI                       "imei"
#define SETTINGS_IMSI                       "imsi"
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
#define SETTINGS_LAST_SYNC                  "lastsync"
#define SETTINGS_SYNC_FREQ                  "syncfreq"
#define SETTINGS_START_ON_BOOT              "startonboot"
#define SETTINGS_ENTER_IS_SEND              "enterissend"
#define SETTINGS_VOICE_CODEC                "voicecodec"

#define SETTINGS_WHATSNEW                   "whatsnew"

// Default settings values

#define DEFAULT_PORT                        443
#define DEFAULT_SHOW_NICKNAMES              false
#define DEFAULT_SHOW_NUMBERS                true
#define DEFAULT_POPUP_ON_FIRST_MESSAGE      false
#define DEFAULT_AUTOMATIC_DOWNLOAD          100
#define DEFAULT_IMPORT_TO_GALLERY           false
#define DEFAULT_SYNC_FREQ                   2
#define DEFAULT_START_ON_BOOT               false
#define DEFAULT_ENTER_IS_SEND               true
#define DEFAULT_VOICE_CODEC                 "aac"

// Synchronization options

#define SYNC_ENABLED        "on"
#define SYNC_INTL_ONLY      "intl"
#define SYNC_DISABLED       "off"


// Status

#define DEFAULT_STATUS          "I am using Yappari!"

// Username

#define DEFAULT_USERNAME        "Yappari User"

// Timers

#define MIN_INTERVAL                300000
#define RETRY_LOGIN_INTERVAL         10000
#define CHECK_QUEUE_INTERVAL          1000
#define CHECK_CONNECTION_INTERVAL   360000

// Directories

#define HOME_DIR        "/.yappari/"
#define YAPPARI_DIR     "/Yappari"
#define LOGS_DIR        HOME_DIR"logs/"
#define CONF_DIR        "/.config/scorpius"
#define DEFAULT_DIR     "/MyDocs"
#define CACHE_DIR       HOME_DIR"cache"
#define IMAGES_DIR      "images"
#define VIDEOS_DIR      "videos"
#define AUDIO_DIR       "sounds"
#define PHOTOS_DIR      "photos"
#define START_FILE      HOME_DIR"start"

#define VOICEPLAYED     "<img src=\"/usr/share/yappari/icons/17x13/yappari-voiceplayed.png\" />"
#define GRAYCHECK       "<img src=\"/usr/share/yappari/icons/17x13/yappari-graycheck.png\" />"
#define CHECK           "<img src=\"/usr/share/yappari/icons/17x13/yappari-greencheck.png\" />"
#define DOUBLECHECK     "<img src=\"/usr/share/yappari/icons/17x13/yappari-greendoublecheck.png\" />"
#define UPLOADING       "<img src=\"/usr/share/yappari/icons/17x13/yappari-uploading.png\" />"

// Extensions

#define EXTENSIONS_VIDEO   "Videos (*.avi *.mpg *.mpeg *.mpe *.3gp *.wmv *.mp4 *.mov *.qt *.AVI *.MPG *.MPEG *.MPE *.3GP *.WMV *.MP4 *.MOV *.QT)"
#define EXTENSIONS_IMAGE   "Images (*.png *.jpg *.jpeg *.gif *.PNG *.JPG *.JPEG *.GIF)"
#define EXTENSIONS_AUDIO   "Audio Files (*.aac *.mp3 *.m4a *.wma *.wav *.ogg *.AAC *.MP3 *.M4A *.WMA *.WAV *.OGG)"

// Image sizes

#define PREVIEW_WIDTH      224
#define PREVIEW_HEIGHT     224

// Yappari URL

#define YAPPARI_URL_BASE    "https://dl.dropboxusercontent.com/u/3392337/Yappari/"
#define VERSION_CHECK_URL   "version.txt"
#define WHATS_NEW_URL       "changelog.txt"
#define DONATE_URL          "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=NVDVRMP4WTFJU&lc=US&currency_code=USD&bn=PP%2dDonationsBF%3abtn_donate_LG%2egif%3aNonHosted"
#define MAGIC_NUMBER        824177453

// Codecs

#define AAC                 "aac"
#define AMR                 "amr"

#endif // GLOBALCONSTANTS_H
