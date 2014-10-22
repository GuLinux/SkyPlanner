#ifndef SKYOBJECTSRESOURCE_H
#define SKYOBJECTSRESOURCE_H

#include <utils/d_ptr.h>
#include <Wt/WResource>
#include "session.h"
class SkyPlanner;
class SkyObjectsResource : public Wt::WResource {
public:
    explicit SkyObjectsResource(Wt::WObject *parent = 0);
    ~SkyObjectsResource();
    virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
private:
    D_PTR;
};

#endif // SKYOBJECTSRESOURCE_H


