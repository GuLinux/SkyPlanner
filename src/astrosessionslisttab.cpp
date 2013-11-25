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

#include "astrosessionslisttab.h"
#include "private/astrosessionslisttab_p.h"
#include "utils/d_ptr_implementation.h"
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include "Models"
#include <Wt/WLineEdit>
#include <Wt/WDateEdit>
#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WMessageBox>
#include <Wt/Auth/Login>
using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroSessionsListTab::Private::Private(Session &session, AstroSessionsListTab* q) : session(session), q(q)
{
}

AstroSessionsListTab::~AstroSessionsListTab()
{
}

AstroSessionsListTab::AstroSessionsListTab(Session &session, Wt::WContainerWidget* parent)
    : d(session, this)
{
  WLineEdit *newSessionName = WW<WLineEdit>();
  newSessionName->setEmptyText("Name");
  WDateEdit *newSessionDate = WW<WDateEdit>();
  newSessionDate->setEmptyText("When");
  newSessionDate->setDate(WDate::currentDate());
  setMinimumSize(WLength::Auto, 500);
  
  WPushButton *newSessionAdd = WW<WPushButton>("Add").css("btn btn-primary").onClick([=](WMouseEvent){
   if(!d->session.login().loggedIn() || ! d->session.user()) return;
    Dbo::Transaction t(d->session);
    d->session.user().modify()->astroSessions().insert(new AstroSession( newSessionName->text().toUTF8(), WDateTime{newSessionDate->date()} ));
    t.commit();
    d->populateSessions();
  });
  addWidget(WW<WContainerWidget>().css("form-inline").add(newSessionName).add(newSessionDate).add(newSessionAdd));
  addWidget(d->sessionsTable = new WTable);
  d->populateSessions();
  d->session.login().changed().connect([=](_n6){ d->populateSessions(); });
}


void AstroSessionsListTab::Private::populateSessions()
{
    Dbo::Transaction t(session);
    sessionsTable->clear();
    sessionsTable->elementAt(0,0)->addWidget(new WText{"Name"});
    sessionsTable->elementAt(0,1)->addWidget(new WText{"Date"});
     if(!session.login().loggedIn() || ! session.user()) return;
     for(auto astroSession: session.user()->astroSessions()) {
       WTableRow *row = sessionsTable->insertRow(sessionsTable->rowCount());
       row->elementAt(0)->addWidget(new WText{astroSession->name()});
       row->elementAt(1)->addWidget(new WText{astroSession->wDateWhen().date().toString()});
       row->elementAt(2)->addWidget(WW<WPushButton>("Remove").css("btn btn-danger").onClick([=](WMouseEvent){
	 WMessageBox *confirm = new WMessageBox("Confirm removal", "Are you sure? This cannot be undone", Wt::Question, Ok | Cancel);
	 confirm->show();
	 confirm->buttonClicked().connect([=](StandardButton b, _n5) {
	   if(b != Wt::Ok) {
	     confirm->reject();
	     return;
	   }
	   confirm->accept();
           Dbo::Transaction t(session);
	   session.user().modify()->astroSessions().erase(astroSession);
	   t.commit();
	   populateSessions();
	});
      }));
     }
}
