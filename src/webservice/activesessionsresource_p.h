#ifndef ACTIVESESSIONSRESOURCE_P_H
#define ACTIVESESSIONSRESOURCE_P_H

#include "activesessionsresource.h"
#include "skyplanner.h"

class ActiveSessionsResource::Private
{
public:
    Private(std::vector<SkyPlanner*> sessions, const std::string &password, ActiveSessionsResource *q);
    std::vector<SkyPlanner*> sessions;
    const std::string password;
private:
    ActiveSessionsResource *q;
};

#endif // ACTIVESESSIONSRESOURCE_H


