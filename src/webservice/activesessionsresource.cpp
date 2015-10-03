/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
#include "activesessionsresource.h"
#include "activesessionsresource_p.h"

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
    : WResource(parent), dptr(sessions, password, this)
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
