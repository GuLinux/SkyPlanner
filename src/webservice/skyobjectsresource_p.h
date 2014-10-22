#ifndef SKYOBJECTSRESOURCE_P_H
#define SKYOBJECTSRESOURCE_P_H

#include "skyobjectsresource.h"
#include "skyplanner.h"

class SkyObjectsResource::Private
{
public:
    Private(Session &session, SkyObjectsResource *q);
    Session &session;
private:
    SkyObjectsResource *q;
};

#endif // SKYOBJECTSRESOURCE_H


