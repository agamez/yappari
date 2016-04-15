#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "dconf/mdconfagent.h"

class AppSettings : public MDConfAgent
{
public:
    explicit AppSettings(QObject *parent = 0);
    static AppSettings *GetInstance(QObject *parent = 0);
};

#endif // APPSETTINGS_H
