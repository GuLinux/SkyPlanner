#include "activesessionsresource.h"
#include "activesessionsresource_p.h"

#include <utils/d_ptr_implementation.h>
#include <Wt/Http/Response>
#include <Wt/Json/Object>
#include <Wt/Json/Array>
#include <Wt/Json/Value>
#include <Wt/Json/Serializer>

using namespace Wt;
using namespace std;
ActiveSessionsResource::Private::Private(std::vector<SkyPlanner *> &sessions, const string &password, ActiveSessionsResource *q)
    : sessions(sessions), password(password), q(q)
{
}

ActiveSessionsResource::ActiveSessionsResource(vector<SkyPlanner *> &sessions, const string &password, WObject *parent)
    : WResource(parent), d(sessions, password, this)
{
}

ActiveSessionsResource::~ActiveSessionsResource()
{
}

struct ActiveSessions : public WtCommons::Json::Object {
  std::vector<SkyPlanner::SessionInfo> sessionInfos;
  WtCommons::Json::Array<SkyPlanner::SessionInfo, WtCommons::Json::Vector, WtCommons::Json::ObjectValue> jsonArray;
  
  ActiveSessions() : jsonArray(sessionInfos) {}
    virtual void add_to_json(Wt::Json::Object& object) const {
      object["sessions"] = jsonArray.toWtValue();
      object["sessions_count"] = static_cast<long long>(sessionInfos.size());
    }
};

void ActiveSessionsResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    auto password = request.getParameter("pwd");
    if(d->password.empty() || !password || d->password != *password) {
      response.setStatus(403);
      response.out() << "403 Forbidden";
      return;
    }
    ActiveSessions activeSessions;
    transform(begin(d->sessions), end(d->sessions), back_inserter(activeSessions.sessionInfos), [](SkyPlanner* a){ return a->sessionInfo(); });
    response.out() << activeSessions.toJson();
    response.setStatus(200);
}
