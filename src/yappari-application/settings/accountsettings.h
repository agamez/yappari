#ifndef ACCOUNTSETTINGS_H
#define ACCOUNTSETTINGS_H

#include "dconf/mdconfagent.h"

class AccountSettings : public MDConfAgent
{
    Q_OBJECT
public:
    explicit AccountSettings(QObject *parent = 0);
    static AccountSettings *GetInstance(QObject *parent = 0);

};

#endif // ACCOUNTSETTINGS_H
