# Copyright 2012 Naikel Aparicio. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ''AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL EELI REILIN OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation
# are those of the author and should not be interpreted as representing
# official policies, either expressed or implied, of copyright holder.
#


#-------------------------------------------------
#
# Project created by QtCreator 2012-05-02T17:00:35
#
#-------------------------------------------------

QT          += core gui network sql maemo5
CONFIG      += mobility qtdbus network link_pkgconfig
MOBILITY     = systeminfo messaging
PKGCONFIG   += libosso-abook-1.0 libebook-1.2 gtk+-2.0 gstreamer-0.10 libpulse
INCLUDEPATH += /usr/include/libosso-abook-1.0 /usr/include/evolution-data-server-1.4/ /usr/include/gtk-2.0 /usr/include/gstreamer-0.10

TARGET = Yappari
TEMPLATE = app

SOURCES += main.cpp \
    Whatsapp/util/utilities.cpp \
    Whatsapp/util/messagedigest.cpp \
    Whatsapp/util/qtmd5digest.cpp \
    Whatsapp/protocoltreenode.cpp \
    Whatsapp/fmessage.cpp \
    Whatsapp/funstore.cpp \
    Whatsapp/key.cpp \
    Whatsapp/attributelist.cpp \
    Whatsapp/protocoltreenodelist.cpp \
    Whatsapp/bintreenodewriter.cpp \
    Whatsapp/protocoltreenodelistiterator.cpp \
    Whatsapp/attributelistiterator.cpp \
    Whatsapp/connection.cpp \
    Whatsapp/bintreenodereader.cpp \
    client.cpp \
    Whatsapp/ioexception.cpp \
    Whatsapp/protocolexception.cpp \
    Whatsapp/warequest.cpp \
    Whatsapp/waexistsrequest.cpp \
    Whatsapp/phonereg.cpp \
    Whatsapp/wacoderequest.cpp \
    Whatsapp/smslistener.cpp \
    Whatsapp/waregrequest.cpp \
    Gui/mainwindow.cpp \
    Gui/chatwindow.cpp \
    Gui/conversationdelegate.cpp \
    Contacts/contactroster.cpp \
    Contacts/contactitem.cpp \
    Gui/selectcontactdialog.cpp \
    Dbus/dbusif.cpp \
    Dbus/dbusifadaptor.cpp \
    Dbus/dbusappletif.cpp \
    Gui/registrationwindow.cpp \
    Gui/phonenumberwidget.cpp \
    Gui/registrationprogresswidget.cpp \
    Whatsapp/phoneregreply.cpp \
    Dbus/notifyobject.cpp \
    Gui/aboutdialog.cpp \
    Dbus/dbusnokiahildonsvndif.cpp \
    Contacts/contact.cpp \
    Contacts/contactlist.cpp \
    Whatsapp/util/datetimeutilities.cpp \
    Dbus/dbusnotificationsif.cpp \
    Dbus/dbusnokiamcesignalif.cpp \
    Sql/chatlogger.cpp \
    Gui/chatarea.cpp \
    Contacts/group.cpp \
    Gui/groupwindow.cpp \
    Whatsapp/exception.cpp \
    Whatsapp/loginexception.cpp \
    Contacts/contactsyncer.cpp \
    Gui/contactselectionmodel.cpp \
    Gui/contactdisplaydelegate.cpp \
    Gui/contactdisplayitem.cpp \
    Sql/rosterdbmanager.cpp \
    Gui/selectemojiwidget.cpp \
    Gui/emojibutton.cpp \
    Gui/changeusernamedialog.cpp \
    Gui/chatimageitem.cpp \
    Whatsapp/formdata.cpp \
    Whatsapp/keystream.cpp \
    Whatsapp/rc4.cpp \
    Whatsapp/util/qthmacsha1.cpp \
    Whatsapp/util/qtrfc2898.cpp \
    Whatsapp/mediaupload.cpp \
    Whatsapp/multipartuploader.cpp \
    Sql/conversationsdb.cpp \
    Gui/chatdisplayitem.cpp \
    Gui/chatdisplaydelegate.cpp \
    Gui/mutedialog.cpp \
    Gui/globalsettingsdialog.cpp \
    Gui/voiceregistrationwidget.cpp \
    Gui/chattextitem.cpp \
    qt-json/json.cpp \
    Gui/whatsnewwindow.cpp \
    Whatsapp/httprequestv2.cpp \
    Whatsapp/mediadownload.cpp \
    Dbus/dbusnokiaimageviewerif.cpp \
    Dbus/dbusnokiamediaplayerif.cpp \
    Gui/mediaselectdialog.cpp \
    Gui/accountinfowindow.cpp \
    Whatsapp/util/datacounters.cpp \
    Gui/networkusagewindow.cpp \
    Gui/profilewindow.cpp \
    Gui/chattextedit.cpp \
    Gui/statuswindow.cpp \
    Contacts/contactlistiterator.cpp \
    Gui/contactinfowindow.cpp \
    Dbus/dbusnokiacsdcallif.cpp \
    Gui/profilepicturewindow.cpp \
    Gui/creategroupwindow.cpp \
    Gui/selectgroupparticipantswindow.cpp \
    Gui/groupparticipantitem.cpp \
    Gui/groupparticipantdelegate.cpp \
    Gui/groupinfowindow.cpp \
    Gui/groupsubjectwindow.cpp \
    Gui/blockedcontactswindow.cpp \
    Multimedia/audioplayer.cpp \
    Multimedia/audiorecorder.cpp

OTHER_FILES += \
    qtc_packaging/debian_fremantle/rules \
    qtc_packaging/debian_fremantle/README \
    qtc_packaging/debian_fremantle/copyright \
    qtc_packaging/debian_fremantle/control \
    qtc_packaging/debian_fremantle/compat \
    qtc_packaging/debian_fremantle/changelog

HEADERS += \
    Whatsapp/util/utilities.h \
    Whatsapp/util/messagedigest.h \
    Whatsapp/util/qtmd5digest.h \
    Whatsapp/protocoltreenode.h \
    Whatsapp/fmessage.h \
    Whatsapp/funstore.h \
    Whatsapp/key.h \
    Whatsapp/attributelist.h \
    Whatsapp/protocoltreenodelist.h \
    Whatsapp/bintreenodewriter.h \
    Whatsapp/protocoltreenodelistiterator.h \
    Whatsapp/attributelistiterator.h \
    Whatsapp/bintreenodereader.h \
    Whatsapp/connection.h \
    client.h \
    Whatsapp/ioexception.h \
    Whatsapp/protocolexception.h \
    Whatsapp/warequest.h \
    Whatsapp/waexistsrequest.h \
    Whatsapp/phonereg.h \
    Whatsapp/wacoderequest.h \
    Whatsapp/smslistener.h \
    Whatsapp/waregrequest.h \
    Contacts/contactitem.h \
    Contacts/contactroster.h \
    Gui/conversationdelegate.h \
    Gui/chatwindow.h \
    Dbus/dbusifadaptor.h \
    Dbus/dbusif.h \
    Dbus/dbusappletif.h \
    Gui/mainwindow.h \
    Gui/registrationwindow.h \
    Gui/phonenumberwidget.h \
    Gui/registrationprogresswidget.h \
    Whatsapp/phoneregreply.h \
    Gui/selectcontactdialog.h \
    Dbus/notifyobject.h \
    Gui/aboutdialog.h \
    Dbus/gtkbindings-new.h \
    Dbus/dbusnokiahildonsvndif.h \
    Contacts/contact.h \
    Contacts/contactlist.h \
    Whatsapp/util/datetimeutilities.h \
    Dbus/dbusnotificationsif.h \
    Dbus/dbusnokiamcesignalif.h \
    Sql/chatlogger.h \
    Gui/chatarea.h \
    Contacts/group.h \
    Gui/groupwindow.h \
    Whatsapp/exception.h \
    Whatsapp/loginexception.h \
    version.h \
    globalconstants.h \
    Contacts/contactsyncer.h \
    Gui/contactselectionmodel.h \
    Gui/contactdisplaydelegate.h \
    Gui/contactdisplayitem.h \
    Sql/rosterdbmanager.h \
    Gui/selectemojiwidget.h \
    Gui/emojibutton.h \
    Gui/changeusernamedialog.h \
    Gui/chatimageitem.h \
    Whatsapp/formdata.h \
    Whatsapp/keystream.h \
    Whatsapp/rc4.h \
    Whatsapp/util/qthmacsha1.h \
    Whatsapp/util/qtrfc2898.h \
    Whatsapp/mediaupload.h \
    Whatsapp/multipartuploader.h \
    Sql/conversationsdb.h \
    Gui/chatdisplayitem.h \
    Gui/chatdisplaydelegate.h \
    Gui/mutedialog.h \
    Gui/globalsettingsdialog.h \
    Gui/voiceregistrationwidget.h \
    Gui/chattextitem.h \
    Gui/whatsnewwindow.h \
    Whatsapp/httprequestv2.h \
    Whatsapp/mediadownload.h \
    Dbus/dbusnokiaimageviewerif.h \
    Dbus/dbusnokiamediaplayerif.h \
    Gui/mediaselectdialog.h \
    Gui/accountinfowindow.h \
    Whatsapp/util/datacounters.h \
    Gui/networkusagewindow.h \
    Gui/profilewindow.h \
    Gui/chattextedit.h \
    Gui/statuswindow.h \
    Contacts/contactlistiterator.h \
    Gui/contactinfowindow.h \
    Dbus/dbusnokiacsdcallif.h \
    Gui/profilepicturewindow.h \
    Gui/creategroupwindow.h \
    Gui/selectgroupparticipantswindow.h \
    Gui/groupparticipantitem.h \
    Gui/groupparticipantdelegate.h \
    Gui/groupinfowindow.h \
    Gui/groupsubjectwindow.h \
    Gui/blockedcontactswindow.h \
    Multimedia/audioplayer.h \
    Multimedia/audiorecorder.h

FORMS += \
    Gui/ui/mainwindow.ui \
    Gui/ui/chatwindow.ui \
    Gui/ui/selectcontactdialog.ui \
    Gui/ui/phonenumberwidget.ui \
    Gui/ui/registrationprogresswidget.ui \
    Gui/ui/aboutdialog.ui \
    Gui/ui/groupsubjectdialog.ui \
    Gui/ui/changestatusdialog.ui \
    Gui/ui/selectemojiwidget.ui \
    Gui/ui/changeusernamedialog.ui \
    Gui/ui/chatimageitem.ui \
    Gui/ui/mutedialog.ui \
    Gui/ui/globalsettingsdialog.ui \
    Gui/ui/voiceregistrationwidget.ui \
    Gui/ui/whatsnewwindow.ui \
    Gui/ui/mediaselectdialog.ui \
    Gui/ui/accountinfowindow.ui \
    Gui/ui/networkusagewindow.ui \
    Gui/ui/profilewindow.ui \
    Gui/ui/statuswindow.ui \
    Gui/ui/contactinfowindow.ui \
    Gui/ui/profilepicturewindow.ui \
    Gui/ui/creategroupwindow.ui \
    Gui/ui/selectgroupparticipantswindow.ui \
    Gui/ui/groupinfowindow.ui \
    Gui/ui/groupsubjectwindow.ui \
    Gui/ui/blockedcontactswindow.ui

maemo5 {
  #VARIABLES
  #isEmpty(PREFIX) {
    #PREFIX = /usr
  #}

  PREFIX = /opt/maemo/usr

  BINDIR = $$PREFIX/bin
  DATADIR = /usr/share
  SHAREDIR = $$PREFIX/share

  DEFINES += DATADIR=\\\"$$DATADIR\\\" PKGDATADIR=\\\"$$PKGDATADIR\\\"

  #MAKE INSTALL

  INSTALLS += target desktop service icon64 icon48 icon16 sharedicons countries script

  target.path = $$BINDIR

  desktop.path = $$DATADIR/applications/hildon
  desktop.files += $${TARGET}.desktop

  service.path = $$DATADIR/dbus-1/services
  service.files += $${TARGET}.service

  icon16.path = $$DATADIR/icons/hicolor/16x16/apps
  icon16.files += icons/16x16/yappari.png

  icon48.path = $$DATADIR/icons/hicolor/48x48/apps
  icon48.files += icons/48x48/yappari.png \
                  icons/48x48/yappari-notification.png

  icon64.path = $$DATADIR/icons/hicolor/64x64/apps
  icon64.files += icons/64x64/yappari.png

  sharedicons.path = $$SHAREDIR/yappari/icons
  sharedicons.files += icons/*

  countries.path = $$SHAREDIR/yappari
  countries.files += share/countries.tab share/mime-types.tab share/whatsnew.txt

  script.path = /etc/event.d
  script.files += event.d/yappari
}





















































































