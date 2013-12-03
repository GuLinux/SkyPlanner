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
    : d(session, this)
{
  WTabWidget *tabs = new WTabWidget(this);
  auto astroSessionsListTab = new AstroSessionsListTab(session);
  astroSessionsListTab->deletingSession().connect([=](const Dbo::ptr<AstroSession> &astroSession, _n5){
    for(auto tab: d->tabs) {
      if(tab.first == astroSession) {
	tabs->removeTab(tab.second);
	delete tab.second;
	d->tabs.erase(tab.first);
      }
    }
  });
  astroSessionsListTab->sessionClicked().connect([=](const Dbo::ptr<AstroSession> &astroSession, _n5){
    if(d->tabs.count(astroSession) == 0) {
      auto astroSessionTab = new AstroSessionTab(astroSession, d->session);
      WMenuItem *newTab = tabs->addTab(astroSessionTab, astroSession->name());
      astroSessionTab->nameChanged().connect([=](const string &newName,_n5){
	newTab->setText(WString::fromUTF8(newName));
	astroSessionsListTab->reload();
      });
      newTab->setCloseable(true);
      d->tabs[astroSession] = astroSessionTab;
    }
    tabs->setCurrentWidget(d->tabs[astroSession]);
  });
  tabs->addTab(astroSessionsListTab, "Sessions List");
  tabs->tabClosed().connect([=](int tabNumber, _n5){
    for(auto tab: d->tabs) {
      if(tab.second == tabs->widget(tabNumber)) {
        d->tabs.erase(tab.first);
      }
    }
    tabs->setCurrentWidget(astroSessionsListTab);
  });
}
