#include "skyobjectsresource_p.h"

#include <utils/d_ptr_implementation.h>
#include "Models"
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
  if(request.pathInfo().empty() || request.pathInfo() == "/") {
    auto objects = session.find<NgcObject>().resultList();
    WtCommons::Json::Array<NgcObjectPtr, Wt::Dbo::collection, WtCommons::Json::PointerObjectConverter<NgcObject, Wt::Dbo::ptr>> jsonArray(objects);
    response.out() << jsonArray.toJson();
    response.setStatus(200);
  }
  response.setStatus(404);
}
