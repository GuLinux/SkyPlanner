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

#include "skyplanner.h"
#include "private/skyplanner_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/utils.h"
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
#include <Wt/Auth/AbstractUserDatabase>
#include "usersettingspage.h"
#include "sendfeedbackpage.hpp"
#include <Wt/WCombinedLocalizedStrings>
#include <Wt/WMessageResourceBundle>
#include <Wt-Commons/whtmltemplateslocalizedstrings.h>
#include "homepage.h"


using namespace std;
using namespace Wt;
using namespace WtCommons;

SkyPlanner::Private::Private( SkyPlanner *q ) : q( q )
{
}

SkyPlanner::~SkyPlanner()
{
}


SkyPlanner *SkyPlanner::instance()
{
  return dynamic_cast<SkyPlanner*>(wApp);
}


const string SkyPlanner::HOME_PATH = "/home/";

SkyPlanner::SkyPlanner( const WEnvironment &environment )
  : WApplication( environment ), d( this )
{
  string stringsDirectory = (boost::filesystem::current_path() / "strings").string();
  readConfigurationProperty("strings_directory", stringsDirectory);

  WCombinedLocalizedStrings *combinedLocalization = new WCombinedLocalizedStrings();
  WMessageResourceBundle *bundle = new WMessageResourceBundle();
  bundle->use(stringsDirectory + "/strings");
  bundle->use(stringsDirectory + "/wt_auth_strings");
  WHTMLTemplatesLocalizedStrings *htmlTemplates = new WHTMLTemplatesLocalizedStrings(stringsDirectory + "/html");
  combinedLocalization->add(bundle);
  combinedLocalization->add(htmlTemplates);
  setLocalizedStrings(combinedLocalization);

  setTitle(WString::tr("application_title"));
  enableUpdates(true);
  setTheme( new WBootstrapTheme( this ) );
  requireJQuery("http://codeorigin.jquery.com/jquery-1.8.3.min.js");

  WNavigationBar *navBar = WW<WNavigationBar>( root() ).addCss( "navbar-inverse" );
  navBar->setResponsive( true );
  navBar->setTitle( WString::tr("application_title"), WLink(WLink::InternalPath, HOME_PATH) );
  useStyleSheet( "/skyplanner_style.css" );
  root()->addWidget(d->notifications = new WContainerWidget);
  d->widgets = new WStackedWidget( root() );
  d->widgets->setTransitionAnimation({WAnimation::AnimationEffect::Fade});
  d->widgets->setMargin(10);
  WMenu *navBarMenu = new WMenu(d->widgets);
  navBar->addMenu(navBarMenu);
  Auth::AuthWidget *authWidget = new Auth::AuthWidget( Session::auth(), d->session.users(), d->session.login() );
  authWidget->model()->addPasswordAuth( &Session::passwordAuth() );
  authWidget->model()->addOAuth(Session::oAuth());
  authWidget->setRegistrationEnabled( true );
  authWidget->processEnvironment();
  
  navBarMenu->setInternalPathEnabled("/");
  
  WMenuItem *home = navBarMenu->addItem(WString::tr("mainmenu_home"), new HomePage(d->session));
  home->setPathComponent("home/");
  
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
  
  WMenuItem *userSettingsMenuItem = navBarMenu->addItem(WString::tr("mainmenu_my_settings"), new UserSettingsPage(d->session));
  d->loggedInItems.push_back(userSettingsMenuItem);
  userSettingsMenuItem->setPathComponent("settings/");

  WMenuItem *feedbackMenuItem = navBarMenu->addItem(WString::tr("mainmenu_feedback"), new SendFeedbackPage(d->session));
  d->loggedInItems.push_back(feedbackMenuItem);
  feedbackMenuItem->setPathComponent("feedback/");

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
  
  auto loginLogoutMessage = [=] {
    if(d->session.login().loggedIn()) {
      Dbo::Transaction t(d->session);
      d->loginname = d->session.authInfo()->identity("loginname");
    }
    spLog("notice") << "***** "
                        << (d->session.login().loggedIn() ? "LOGIN"  : "LOGOUT")
                        << ": user " << d->loginname << " logged "
                        << (d->session.login().loggedIn() ?   "in"  : "out")
                        << "; wApp->sessionId " << sessionId();
  };
  
  logout->triggered().connect([=](WMenuItem*,_n5){ d->session.login().logout(); });
  d->session.login().changed().connect([=](_n6){
    if(internalPathMatches("/login") || internalPathMatches("/logout"))
      setInternalPath(HOME_PATH, true);
    setMenuItemsVisibility();
    loginLogoutMessage();
  });
  setMenuItemsVisibility();
  if(d->session.login().loggedIn())
    loginLogoutMessage();
  auto handlePath = [=](const string &newPath){
    spLog("notice") << __PRETTY_FUNCTION__ << ": newPath=" << newPath;
    if(internalPathMatches("/dss")) {
      d->loadDSSPage(internalPathNextPart("/dss/"));
    }
    if(internalPathMatches("/sessions")) {
      astrosessionspage->open(internalPathNextPart("/sessions/"));
    }
    d->previousInternalPath = newPath;
  };
  internalPathChanged().connect([=](string p, ...) {handlePath(p); });
  d->widgets->addWidget(d->dssContainer = new WContainerWidget);
  if(!d->session.login().loggedIn() && ! internalPathMatches("/dss") ) {
    setInternalPath(HOME_PATH, true);
  }
  handlePath(internalPath());

  root()->addWidget(WW<WContainerWidget>().css("alert blog-link-cell").add(
                        WW<WAnchor>("http://blog.gulinux.net", "Blog: gulinux.net").setTarget(TargetNewWindow)
                        ));
}

void SkyPlanner::Private::loadDSSPage( const std::string &hexId )
{
  WWidget *currentWidget = widgets->currentWidget();
  dssContainer->clear();
  auto objectId = Utils::fromHexString<Dbo::dbo_traits<NgcObject>::IdType>(hexId);
  Dbo::Transaction t(session);
  NgcObjectPtr ngcObject = session.find<NgcObject>().where("id = ?").bind(objectId);
  string previousPath = previousInternalPath;
  DSSPage *dssPage = new DSSPage(ngcObject, session, [=]{
    widgets->setCurrentWidget( currentWidget );
    wApp->setInternalPath( previousPath, true );
    dssContainer->clear();
  });
  dssContainer->addWidget( dssPage );
  widgets->setCurrentWidget( dssContainer );
}

WLogEntry SkyPlanner::uLog(const string &type) const
{
    return WApplication::log(type) << " - user: '" << d->loginname << "' - ";
}


SkyPlanner::Notification *SkyPlanner::notification(const WString &title, const WString &content, Notification::Type type, int autoHideSeconds, WContainerWidget *addTo)
{
  Notification *notification = new Notification(title, content, type, autoHideSeconds);
  (addTo ? addTo : d->notifications)->addWidget(notification);
  notification->animateShow({WAnimation::Fade, WAnimation::EaseInOut, 500});
  return notification;
}

class SkyPlanner::Notification::Private {
public:
  Signal<> closed;
};

Signal<> &SkyPlanner::Notification::closed() const
{
  return d->closed;
}

SkyPlanner::Notification::Notification(const WString &title, const WString &content, Type type, int autoHideSeconds, WContainerWidget *parent)
  : WContainerWidget(parent), d()
{
  static map<Type,string> notificationStyles {
    {Error, "alert-error"},
    {Success, "alert-success"},
    {Information, "alert-info"},
  };
  addStyleClass("alert");
  addStyleClass("alert-block");
  addStyleClass(notificationStyles[type]);
  auto deleteNotification = [=](WMouseEvent) {
    d->closed.emit();
    hide();
    WTimer::singleShot(3000, [=](WMouseEvent){delete this; });
  };
  if(autoHideSeconds<=0) {
    WPushButton *closeButton = WW<WPushButton>().css("close").onClick(deleteNotification);
    closeButton->setTextFormat(XHTMLUnsafeText);
    closeButton->setText("<h4><strong>&times;</strong></h4>");
    addWidget(closeButton);
  } else {
    WTimer::singleShot(1000*autoHideSeconds, deleteNotification);
  }

  addWidget(new WText{WString("<h4>{1}</h4>").arg(title) });
  addWidget(new WText{content});
}

SkyPlanner::Notification::~Notification()
{

}
