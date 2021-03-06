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
#include "homepage.h"
#include "private/homepage_p.h"
#include "wt_helpers.h"
#include "session.h"
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/Auth/Login>
#include <Wt/WAnchor>
#include "models/Models"
#include "urls.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
HomePage::Private::Private( Session &session, HomePage *q ) : session(session), link(q, "go_to_link"), q( q )
{
}

HomePage::~HomePage()
{
}

HomePage::HomePage( Session &session, Wt::WContainerWidget *parent )
  : WCompositeWidget(parent), dptr( session, this )
{
  setImplementation(d->content = new WTemplate(WString::tr("home")));
  d->populate();
  wApp->internalPathChanged().connect([=](const string &newPath, ...) {
    if(wApp->internalPathMatches("/home"))
      d->populate();
  });
  d->link.connect([=](const string &path, ...) { wApp->setInternalPath(path, true);});
}

void HomePage::Private::populate() {
  Dbo::Transaction t(session);
  content->clear();
  content->addFunction("tr", &WTemplate::Functions::tr);
  content->setCondition("logged-in", session.login().loggedIn());
  content->setCondition("not-logged-in", !session.login().loggedIn());
  if(session.login().loggedIn()) {
    content->bindString("login-name", session.user()->loginName() );
  }
  content->bindString("login-menu-path", link.createCall("'/login/'"));
  content->bindString("sessions-menu-path", link.createCall("'/sessions/list/'"));
  content->bindString("logo-image", URLs::skyplanner_logo);

  auto catalogues = session.find<Catalogue>().where("hidden < ?").bind(0xFF).orderBy("priority ASC").resultList();
  content->setCondition("have-features-list", catalogues.size() > 0);
  WContainerWidget *cataloguesWidget = WW<WContainerWidget>().css("container-fluid");
  cataloguesWidget->setList(true);
  content->bindWidget("included-catalogues", cataloguesWidget);
  content->bindInt("sky-objects-count", session.query<int>("select count(id) from objects").resultValue() );
  for(auto catalogue: catalogues) 
    cataloguesWidget->addWidget(WW<WContainerWidget>().css("col-xs-3").add(WW<WText>(WString::fromUTF8(catalogue->name() ))));
}
