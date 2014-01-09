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

#include "astroplanner.h"
#include "private/astroplanner_p.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/WNavigationBar>
#include <Wt/WBootstrapTheme>
#include <Wt/WStackedWidget>
#include <Wt/Auth/AuthWidget>
#include <Wt/WMenu>
#include "Wt-Commons/wt_helpers.h"
#include "telescopespage.h"
#include "astrosessionspage.h"
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/Login>
#include <boost/filesystem.hpp>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WTimer>
#include "models/Models"
#include "widgets/dsspage.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

AstroPlanner::Private::Private( AstroPlanner *q ) : q( q )
{
}

AstroPlanner::~AstroPlanner()
{
}


AstroPlanner *AstroPlanner::instance()
{
  return dynamic_cast<AstroPlanner*>(wApp);
}

WStackedWidget *AstroPlanner::widgetsStack() const
{
  return d->widgets;
}

AstroPlanner::AstroPlanner( const WEnvironment &environment )
  : WApplication( environment ), d( this )
{
  string stringsDirectory = (boost::filesystem::current_path() / "strings").string();
  readConfigurationProperty("strings_directory", stringsDirectory);
  messageResourceBundle().use(stringsDirectory + "/strings");
  messageResourceBundle().use(stringsDirectory + "/wt_auth_strings");
  setTitle(WString::tr("application_title"));
  enableUpdates(true);
  setTheme( new WBootstrapTheme( this ) );
  requireJQuery("http://codeorigin.jquery.com/jquery-1.8.3.min.js");

  cerr << __PRETTY_FUNCTION__ << endl;
  WNavigationBar *navBar = WW<WNavigationBar>( root() ).addCss( "navbar-inverse" );
  navBar->setResponsive( true );
  navBar->setTitle( WString::tr("application_title") );
  useStyleSheet( "/skyplanner_style.css" );
  root()->addWidget(d->notifications = new WContainerWidget);
  d->widgets = new WStackedWidget( root() );
  d->widgets->setTransitionAnimation({WAnimation::AnimationEffect::Fade});
  d->widgets->setMargin(10);
  WMenu *navBarMenu = new WMenu(d->widgets);
  navBar->addMenu(navBarMenu);
  Auth::AuthWidget *authWidget = new Auth::AuthWidget( Session::auth(), d->session.users(), d->session.login() );
  authWidget->model()->addPasswordAuth( &Session::passwordAuth() );
  authWidget->setRegistrationEnabled( true );
  authWidget->processEnvironment();
  
  navBarMenu->setInternalPathEnabled("/");
  WMenuItem *authMenuItem;
  d->loggedOutItems.push_back(authMenuItem = navBarMenu->addItem(WString::tr("mainmenu_login"), authWidget));
  authMenuItem->setPathComponent("login/");
  TelescopesPage *telescopesPage = new TelescopesPage(d->session);
  AstroSessionsPage *astrosessionspage = new AstroSessionsPage(d->session);
  
  WMenuItem *mySessionsMenuItem;
  
  d->loggedInItems.push_back(mySessionsMenuItem = navBarMenu->addItem(WString::tr("mainmenu_my_sessions"), astrosessionspage));
  mySessionsMenuItem->setPathComponent("sessions/");
  WMenuItem *telescopesMenuItem;
  d->loggedInItems.push_back(telescopesMenuItem = navBarMenu->addItem(WString::tr("mainmenu_my_telescopes"), telescopesPage));
  telescopesMenuItem->setPathComponent("telescopes/");
  
  WMenuItem *logout = navBarMenu->addItem(WString::tr("mainmenu_logout"));
  logout->setPathComponent("logout/");
  d->loggedInItems.push_back(logout);
  
  
  
  auto setMenuItemsVisibility = [=] {
    bool loggedIn = d->session.login().loggedIn();
    for(auto i: d->loggedInItems)
      i->setHidden(!loggedIn);
    for(auto i: d->loggedOutItems)
      i->setHidden(loggedIn);
  };
  
  auto setLoggedInWidget = [=] {
    navBarMenu->select(d->session.login().loggedIn() ? mySessionsMenuItem : authMenuItem);
  };
  
  logout->triggered().connect([=](WMenuItem*,_n5){ d->session.login().logout(); });
  d->session.login().changed().connect([=](_n6){
    setMenuItemsVisibility();
    setLoggedInWidget();
  });
  setMenuItemsVisibility();
  setLoggedInWidget();
  
  internalPathChanged().connect([=](const string &newPath, _n5){
    log("notice") << "first folder: " << internalPathNextPart("/");
    if(internalPathMatches("/dss")) {
      d->loadDSSPage(internalPathNextPart("/dss/"));
    }
    d->previousInternalPath = newPath;
  });
}

void AstroPlanner::Private::loadDSSPage( const std::string &hexId )
{
  WWidget *currentWidget = widgets->currentWidget();
  string currentInternalPath = wApp->internalPath();
  stringstream s;
  s << hex << hexId;
  Dbo::dbo_traits<NgcObject>::IdType objectId;
  s >> objectId;
  Dbo::Transaction t(session);
  NgcObjectPtr ngcObject = session.find<NgcObject>().where("id = ?").bind(objectId);
  string previousPath = previousInternalPath;
  DSSPage *dssPage = new DSSPage(ngcObject, session, [=]{
    widgets->setCurrentWidget( currentWidget );
    wApp->setInternalPath( previousPath, true );
  });
  widgets->addWidget( dssPage );
  widgets->setCurrentWidget( dssPage );
}


WContainerWidget *AstroPlanner::notification(const WString &title, const WString &content, NotificationType type, int autoHideSeconds)
{
  static map<NotificationType,string> notificationStyles {
    {Error, "alert-error"},
    {Success, "alert-success"},
    {Information, "alert-information"},
  };
  WContainerWidget *notification = WW<WContainerWidget>().addCss("alert alert-block").addCss(notificationStyles[type]).setHidden(true);
  auto deleteNotification = [=](WMouseEvent) {
    notification->animateHide({WAnimation::Fade, WAnimation::EaseInOut, 500});
    WTimer::singleShot(3000, [=](WMouseEvent){delete notification; });
  };
  if(autoHideSeconds<=0) {
    WPushButton *closeButton = WW<WPushButton>().css("close").onClick(deleteNotification);
    closeButton->setTextFormat(XHTMLUnsafeText);
    closeButton->setText("&times;");
    notification->addWidget(closeButton);
  } else {
    WTimer::singleShot(1000*autoHideSeconds, deleteNotification);
  }
  
  notification->addWidget(new WText{WString("<h4>{1}</h4>").arg(title) });
  notification->addWidget(new WText{content});
  d->notifications->addWidget(notification);
  notification->animateShow({WAnimation::Fade, WAnimation::EaseInOut, 500});
  return notification;
}

