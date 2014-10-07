#ifndef ACTIVESESSIONSRESOURCE_H
#define ACTIVESESSIONSRESOURCE_H

#include <utils/d_ptr.h>
#include <Wt/WResource>

class SkyPlanner;
class ActiveSessionsResource : public Wt::WResource {
public:
    explicit ActiveSessionsResource(std::vector<SkyPlanner*> &sessions, const std::string &password, Wt::WObject *parent = 0);
    ~ActiveSessionsResource();
    virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
private:
    D_PTR;
};

#endif // ACTIVESESSIONSRESOURCE_H


