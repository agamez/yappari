# Applet Stubs
qdbusxml2cpp -v -c DBusAppletIf -p dbusappletif.h:dbusappletif.cpp org.scorpius.YappariApplet.xml
#qdbusxml2cpp -c DBusIfAppletAdaptor -a dbusifappletadaptor.h:dbusifappletadaptor.cpp org.scorpius.YappariApplet.xml
# Main application Stubs
qdbusxml2cpp -v -c DBusIf -p dbusif.h:dbusif.cpp org.scorpius.Yappari.xml
qdbusxml2cpp -c DBusIfAdaptor -a dbusifadaptor.h:dbusifadaptor.cpp org.scorpius.Yappari.xml
# Nokia Stubs
qdbusxml2cpp -v -c DBusNokiaHildonSVNDIf -p dbusnokiahildonsvndif.h:dbusnokiahildonsvndif.cpp com.nokia.HildonSVNotificationDaemon.xml
qdbusxml2cpp -v -c DBusNotificationsIf -p dbusnotificationsif.h:dbusnotificationsif.cpp org.freedesktop.Notifications.xml
qdbusxml2cpp -v -c DBusNokiaMCESignalIf -p dbusnokiamcesignalif.h:dbusnokiamcesignalif.cpp com.nokia.mce.signal.xml
