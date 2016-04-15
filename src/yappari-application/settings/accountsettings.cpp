#include "accountsettings.h"

AccountSettings::AccountSettings(QObject *parent):
    MDConfAgent("/apps/mitakuuluu3/account/", parent)
{
    watchKey("avatar", QString());
    watchKey("pushname", value("login").toString());
    watchKey("message", "Hey there! I am using WhatsApp.");
}

AccountSettings *AccountSettings::GetInstance(QObject *parent)
{
    static AccountSettings* lsSingleton = NULL;
    if (!lsSingleton) {
        lsSingleton = new AccountSettings(parent);
    }
    return lsSingleton;
}
