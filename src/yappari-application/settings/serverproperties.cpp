#include "serverproperties.h"

ServerProperties::ServerProperties(QObject *parent):
    MDConfAgent("/apps/mitakuuluu3/props/", parent)
{
}

ServerProperties *ServerProperties::GetInstance(QObject *parent)
{
    static ServerProperties* lsSingleton = NULL;
    if (!lsSingleton) {
        lsSingleton = new ServerProperties(parent);
    }
    return lsSingleton;
}
