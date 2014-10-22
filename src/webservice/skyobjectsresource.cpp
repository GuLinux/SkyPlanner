#include "skyobjectsresource_p.h"

#include <utils/d_ptr_implementation.h>
#include <Wt/Http/Response>
#include <Wt/Json/Object>
#include <Wt/Json/Array>
#include <Wt/Json/Value>
#include <Wt/Json/Serializer>

using namespace Wt;
using namespace std;
SkyObjectsResource::Private::Private(SkyObjectsResource *q)
    : q(q)
{
}

SkyObjectsResource::SkyObjectsResource(WObject* parent)
    : WResource(parent), d(this)
{
}

SkyObjectsResource::~SkyObjectsResource()
{
}


void SkyObjectsResource::handleRequest(const Http::Request &request, Http::Response &response)
{
  /*
    auto password = request.getParameter("pwd");
    if(d->password.empty() || !password || d->password != *password) {
      response.setStatus(403);
      response.out() << "403 Forbidden";
      return;
    }
    SkyObjects activeSessions;
    transform(begin(d->sessions), end(d->sessions), back_inserter(activeSessions.sessionInfos), [](SkyPlanner* a){ return a->sessionInfo(); });
    response.out() << activeSessions.toJson();
    */
  Session session;
  response.out() << "path: " << request.path() << endl;
  response.out() << "pathinfo: " << request.pathInfo() << endl;
    response.setStatus(200);
}
