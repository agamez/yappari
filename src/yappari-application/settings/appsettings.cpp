#include "appsettings.h"

AppSettings::AppSettings(QObject *parent):
    MDConfAgent("/apps/mitakuuluu3/settings/", parent)
{
    watchKey("sendByEnter", false);
    watchKey("notificationsPreview", true);
}

AppSettings *AppSettings::GetInstance(QObject *parent)
{
    static AppSettings* lsSingleton = NULL;
    if (!lsSingleton) {
        lsSingleton = new AppSettings(parent);
    }
    return lsSingleton;
}
