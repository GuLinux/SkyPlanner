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
#include "astrosessiontab.h"
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
#include <Wt/WLabel>
#include <Wt/Auth/Login>
#include <Wt/Utils>
#include <Wt/WLocalDateTime>
#include "ephemeris.h"
#include "skyplanner.h"
#include "utils/format.h"

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

  WContainerWidget *helpContainer = new WContainerWidget(this);
  auto displayFirstLoginHelp = [=] {
    return; // TODO: reenable when ready
    if(! wApp->internalPathMatches("/sessions") || !d->session.login().loggedIn()) return;
    Dbo::Transaction t(d->session);
    if(User::Setting::value<bool>(t, "astrosessionlisttab_firstintro_shown", d->session.user(), false))
      return;
    helpContainer->clear();
    SkyPlanner::instance()->notification(WString::tr("help_notification"), WString::tr("help_astrosessionlisttab_1"), SkyPlanner::Notification::Information, 0, helpContainer);
    User::Setting::setValue(t, "astrosessionlisttab_firstintro_shown", d->session.user(), true);
  };
  displayFirstLoginHelp();
  wApp->internalPathChanged().connect([=](const string &, ...){ displayFirstLoginHelp(); });
  WLineEdit *newSessionName = WW<WLineEdit>();
  newSessionName->setEmptyText(WString::tr("astrosessionslisttab_name"));
  WDateEdit *newSessionDate = WW<WDateEdit>();
  newSessionDate->setEmptyText(WString::tr("astrosessionslisttab_when"));
  newSessionDate->setDate(WDate::currentDate());
  setMinimumSize(WLength::Auto, 500);
  
  WPushButton *newSessionAdd = WW<WPushButton>(WString::tr("buttons_add")).css("btn btn-primary btn-small").onClick([=](WMouseEvent){
    if(!d->session.login().loggedIn() || ! d->session.user() ) return;
    if( newSessionName->text().empty() ) {
      SkyPlanner::instance()->notification(WString::tr("notification_error_title"), WString::tr("astrosessionslisttab_add_new_name_empty"), SkyPlanner::Notification::Error, 10  );
      return;
    }
    if( ! newSessionDate->date().isValid() ) {
      SkyPlanner::instance()->notification(WString::tr("notification_error_title"), WString::tr("astrosessionslisttab_add_new_date_invalid"), SkyPlanner::Notification::Error, 10  );
      return;
    }
    d->addNew(newSessionName->text(), newSessionDate->date());
    d->populateSessions();
    newSessionName->setText("");
  }).setEnabled(true);
//  newSessionName->keyWentUp().connect([=](WKeyEvent){ newSessionAdd->setEnabled(!newSessionName->text().empty() );});
  addWidget(WW<WContainerWidget>().css("form-inline").add(new WLabel{WString::tr("astrosessionslisttab_add_new_label")}).add(newSessionName).add(newSessionDate).add(newSessionAdd));
  
  vector<pair<Ephemeris::LunarPhase,boost::posix_time::ptime>> newMoons;
  Ephemeris moonPhaseEphemeris{{}};
  for(int i=-0; i<60; i++) {
    auto day = boost::posix_time::second_clock::local_time() + boost::posix_time::time_duration(24*i, 0, 0);
    Ephemeris::LunarPhase phase = moonPhaseEphemeris.moonPhase(day);
    if(phase.illuminated_fraction < 0.15)
      newMoons.push_back({phase, day});
  }
  addWidget(new WText{WString::tr("astrosessionslisttab_next_nights_without_moon")});
  string separator;
  for(uint64_t i=0; i<min(newMoons.size(), static_cast<std::size_t>(5)); i++) {
    if(i>0)
      addWidget(new WText{", "});
    auto date = WDateTime::fromPosixTime(newMoons[i].second).date();
    addWidget(WW<WAnchor>("", WLocalDateTime(date, {0,0,0}).toString("dddd, dd MMMM") ).css("link").setMargin(5, Left).onClick([=](WMouseEvent){
      newSessionDate->setDate(date);
    }));
  }
  
  addWidget(d->sessionsTable = WW<WTable>().addCss("table table-striped table-hover"));
  d->sessionsTable->setHeaderCount(1);
  d->populateSessions();
  d->session.login().changed().connect([=](_n6){ d->populateSessions(); });
}

Dbo::ptr<AstroSession> AstroSessionsListTab::Private::addNew(const Wt::WString &name, const Wt::WDate &date)
{
  Dbo::Transaction t(session);
  Dbo::ptr<AstroSession> astroSession = session.add(new AstroSession( name.toUTF8(), WDateTime{date}, session.user() ));
  return astroSession;
}

void AstroSessionsListTab::reload()
{
  d->populateSessions();
}

void AstroSessionsListTab::Private::populateSessions()
{
    Dbo::Transaction t(session);
    sessionsTable->clear();
    sessionsTable->elementAt(0,0)->addWidget(new WText{WString::tr("astrosessionslisttab_name")});
    sessionsTable->elementAt(0,1)->addWidget(new WText{WString::tr("astrosessionslisttab_when")});
     if(!session.login().loggedIn() || ! session.user()) return;
     for(auto astroSession: session.find<AstroSession>().where("user_id = ?").bind(session.user().id()).orderBy("\"when\" DESC").resultList() ) {
       WTableRow *row = sessionsTable->insertRow(sessionsTable->rowCount());
       row->elementAt(0)->addWidget(WW<WAnchor>(
              WLink(WLink::InternalPath, AstroSessionTab::pathComponent(astroSession, t)), WString::fromUTF8(astroSession->name()))
             .css("link"));
       row->elementAt(1)->addWidget(new WText{WLocalDateTime(astroSession->wDateWhen().date(), astroSession->wDateWhen().time()).toString("dddd, dd MMMM yyyy")});
       row->elementAt(2)->addWidget(WW<WPushButton>(WString::tr("buttons_remove")).css("btn btn-danger btn-mini").onClick([=](WMouseEvent){
	 WMessageBox *confirm = new WMessageBox(WString::tr("messagebox_confirm_removal_title"), WString::tr("messagebox_confirm_removal_message"), Wt::Question, Ok | Cancel);
	 confirm->show();
	 confirm->buttonClicked().connect([=](StandardButton b, _n5) {
	   if(b != Wt::Ok) {
	     confirm->reject();
	     return;
	   }
	   confirm->accept();
	   deletingSession.emit(astroSession);
           Dbo::Transaction t(session);
           for(auto object: astroSession.modify()->astroSessionObjects()) {
               astroSession.modify()->astroSessionObjects().erase(object);
               object.remove();
           }
            session.user().modify()->astroSessions().erase(astroSession);
            Dbo::ptr<AstroSession> s = astroSession;
            s.remove();
            t.commit();
            populateSessions();
	});
      }));
     }
}

Signal< Dbo::ptr< AstroSession > >& AstroSessionsListTab::deletingSession() const
{
  return d->deletingSession;
}
Signal< Dbo::ptr< AstroSession > >& AstroSessionsListTab::sessionClicked() const
{
  return d->sessionClicked;
}

