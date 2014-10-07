#include "activesessionsresource.h"
#include "activesessionsresource_p.h"

#include <utils/d_ptr_implementation.h>
#include <Wt/Http/Response>

using namespace Wt;
using namespace std;
ActiveSessionsResource::Private::Private(vector<SkyPlanner *> sessions, const string &password, ActiveSessionsResource *q)
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

void ActiveSessionsResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    auto password = request.getParameter("pwd");
    if(d->password.empty() || !password || d->password != *password) {
      response.setStatus(403);
      response.out() << "403 Forbidden";
      return;
    }
    response.out() << "Active sessions: " << d->sessions.size() << std::endl;
    for(SkyPlanner *app: d->sessions) {
        SkyPlanner::SessionInfo infos = app->sessionInfo();
        response.out() << "Session " << app->sessionId() << ", started: " << boost::posix_time::to_simple_string(infos.started) << ", ip: "
                       << infos.ipAddress << ", user agent: " << infos.userAgent << ", username: " << infos.username << std::endl;
    }
    response.setStatus(200);
}
