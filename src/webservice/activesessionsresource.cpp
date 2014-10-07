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

// TODO: move into a common place?

Json::Value JsonObject(const std::map<string, Json::Value> &obj) {
    Json::Value v(Json::ObjectType);
    Json::Object &o = v;
    for(auto element: obj)
        o[element.first] = element.second;
    return v;
}

void ActiveSessionsResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    auto password = request.getParameter("pwd");
    if(d->password.empty() || !password || d->password != *password) {
      response.setStatus(403);
      response.out() << "403 Forbidden";
      return;
    }
    Json::Value jsonSessionsValue(Json::ArrayType);
    Json::Array &jsonSessions = jsonSessionsValue;
    Json::Object jsonResponse;
    jsonResponse["sessions-count"] = {static_cast<long long>(d->sessions.size())};
    for(SkyPlanner *app: d->sessions) {
        SkyPlanner::SessionInfo infos = app->sessionInfo();
        jsonSessions.push_back(JsonObject({
            {"session-id", WString::fromUTF8(app->sessionId())},
            {"started", WString::fromUTF8(boost::posix_time::to_iso_extended_string(infos.started))},
            {"ip-address", WString::fromUTF8(infos.ipAddress)},
            {"user-agent", WString::fromUTF8(infos.userAgent)},
            {"username", WString::fromUTF8(infos.username)},
        }));
    }
    jsonResponse["sessions"] = jsonSessionsValue;
    response.out() << Json::serialize(jsonResponse);
    response.setStatus(200);
}
