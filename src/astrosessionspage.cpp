/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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
#include "Models"
#include "astrosessionspage.h"
#include "private/astrosessionspage_p.h"
#include "utils/d_ptr_implementation.h"
#include "Wt-Commons/wt_helpers.h"
#include "session.h"
#include "astrosessionslisttab.h"
#include "astrosessiontab.h"
#include <Wt/WTabWidget>
#include <Wt/WMenuItem>
#include <Wt/WMenu>
#include <Wt/WPushButton>
#include <boost/regex.hpp>
#include "utils/format.h"
#include "utils/utils.h"
#include "skyplanner.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;
AstroSessionsPage::Private::Private(Session& session, AstroSessionsPage* q) : session(session), q(q)
{
}

AstroSessionsPage::~AstroSessionsPage()
{
}

AstroSessionsPage::AstroSessionsPage(Session &session, WContainerWidget* parent)
  : WContainerWidget(parent), d(session, this)
{
  d->tabWidget = new WTabWidget(this);
//  d->tabWidget->setInternalPathEnabled("sessions/");
  d->astroSessionsListTab = new AstroSessionsListTab(session);
  d->astroSessionsListTab->deletingSession().connect([=](const Dbo::ptr<AstroSession> &astroSession, _n5){
    d->removeTab(astroSession);
  });

  WMenuItem *listMenuItem = d->tabWidget->addTab(d->astroSessionsListTab, WString::tr("astrosessionspage_sessions_list"));
  listMenuItem->addStyleClass("hidden-print");
  listMenuItem->setPathComponent("list/");
  d->tabWidget->tabClosed().connect([=](int tabNumber, _n5){
    spLog("notice") << __PRETTY_FUNCTION__ << ": tabNumber=" << tabNumber;
    d->removeTab(d->tabs[0].astroSession);
    d->tabWidget->setCurrentWidget(d->astroSessionsListTab);
  });
  d->tabs[0].path = "/sessions/list";
  d->tabWidget->currentChanged().connect([=](int index, _n5){
    spLog("notice") << __PRETTY_FUNCTION__ << ", index= " << index << "path: " << d->tabs[index].path;
    wApp->setInternalPath(d->tabs[index].path);
  });
}

void AstroSessionsPage::Private::removeTab(const Wt::Dbo::ptr<AstroSession> &astroSession)
{
  for(auto tab: tabs) {
    if(tab.second.astroSession != astroSession)
      continue;
    tabWidget->removeTab(tab.second.page);
    delete tab.second.page;
    //delete tab.second.menuItem;
    tabs.erase(tab.first);
  }
}

void AstroSessionsPage::open(const string &tabName)
{
  spLog("notice") << __PRETTY_FUNCTION__ << ": tabName=" << tabName;
  if(tabName == "list") {
    d->tabWidget->setCurrentIndex(0);
    return;
  }
  auto sessionId = Utils::fromHexString<Dbo::dbo_traits<AstroSession>::IdType>(tabName);
  Dbo::Transaction t(d->session);
  AstroSessionPtr astroSession;
  if(d->session.user()->isAdmin())
    astroSession = d->session.find<AstroSession>().where("id = ?").bind(sessionId);
  else
    astroSession = d->session.find<AstroSession>().where("id = ?").where("user_id = ?").bind(sessionId).bind(d->session.user().id());
  if(!astroSession) {
    d->tabWidget->setCurrentIndex(0);
    spLog("warning") << "Unable to find astroSession for path: " << wApp->internalPath() << ", tabName=" << tabName;
    wApp->setInternalPath("/sessions/list", true);
    return;
  }
  string internalPath = AstroSessionTab::pathComponent(astroSession, t);
  spLog("notice") << "Setting internal path to " << internalPath;
  wApp->setInternalPath(internalPath, false);
  for(auto tab = find_if(begin(d->tabs), end(d->tabs), [=](pair<int, Private::Tab> t){ return t.second.astroSession.id() == sessionId; }); tab != end(d->tabs); tab++) {
    d->tabWidget->setCurrentWidget(tab->second.page);
    return;
  }
  auto astroSessionTab = new AstroSessionTab(astroSession, d->session);
  WMenuItem *newTab = d->tabWidget->addTab(astroSessionTab, WString::fromUTF8(astroSession->name()));
  newTab->addStyleClass("hidden-print");
  //newTab->setPathComponent( AstroSessionTab::pathComponent(astroSession, t) );
  astroSessionTab->nameChanged().connect([=](const string &newName,_n5){
    newTab->setText(WString::fromUTF8(newName));
    d->astroSessionsListTab->reload();
  });
  newTab->setCloseable(true);
  astroSessionTab->close().connect([=](_n6){
    newTab->close();
  });
  //newTab->addWidget(WW<WPushButton>().setTextFormat(Wt::XHTMLUnsafeText).setText("&times;").css("close"));
  d->tabs[d->tabWidget->indexOf(astroSessionTab)] = {internalPath, astroSessionTab, newTab, astroSession};
  d->tabWidget->setCurrentWidget(astroSessionTab);
}
