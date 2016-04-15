#ifndef SERVERPROPERTIES_H
#define SERVERPROPERTIES_H

#include "dconf/mdconfagent.h"

class ServerProperties : public MDConfAgent
{
    Q_OBJECT
public:
    explicit ServerProperties(QObject *parent = 0);
    static ServerProperties *GetInstance(QObject *parent = 0);

};

#endif // SERVERPROPERTIES_H
