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
#include "widgets/astroobjectstable.h"
#include <Wt/Auth/AbstractUserDatabase>
#include "usersettingspage.h"
#include "sendfeedbackpage.hpp"
#include <Wt/WCombinedLocalizedStrings>
#include <Wt/WMessageResourceBundle>
#include <Wt/Dbo/QueryModel>
#include <Wt-Commons/whtmltemplateslocalizedstrings.h>
#include "homepage.h"
#include <mutex>
#include <Wt/WProgressBar>
#include <Wt/WPopupMenu>
#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WProgressBar>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WMessageBox>
#include "utils/format.h"
#include "astrosessiontab.h"
#include <boost/algorithm/string.hpp>


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
  addMetaHeader("viewport", "width=device-width, initial-scale=1, maximum-scale=1");

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
  auto theme = new WBootstrapTheme(this);
  theme->setVersion(WBootstrapTheme::Version3);
  setTheme( theme );
  requireJQuery("http://codeorigin.jquery.com/jquery-1.8.3.min.js");
  {
    Dbo::Transaction t(d->session);
    long objectsWithoutConstellationSize = d->session.query<long>("select count(*) from objects where constellation_abbrev is null").resultValue();
    if(objectsWithoutConstellationSize > 0) {
      spLog("notice") << "Found " << objectsWithoutConstellationSize << " objects without constellation, running update";
      static mutex updateDatabaseMutex;
      unique_lock<mutex> lockUpdateDatabase(mutex);
      long current = 0;
      int progressPercent = 0;
      for(auto object: d->session.find<NgcObject>().where("constellation_abbrev is null").resultList()) {
        object.modify()->updateConstellation();
        object.flush();
        int currentProgress = 100. * static_cast<double>(++current) / static_cast<double>(objectsWithoutConstellationSize);
        if(currentProgress > progressPercent) {
          spLog("notice") << "Updated object " << current << " of " << objectsWithoutConstellationSize << ", " << currentProgress << "%";
          progressPercent = currentProgress;
        }
      }
    }
  }

  WNavigationBar *navBar = WW<WNavigationBar>(root()).addCss( "navbar-inverse" );

  navBar->setResponsive( true );
  navBar->setTitle( WString::tr("application_title"), WLink(WLink::InternalPath, HOME_PATH) );
  useStyleSheet( "/skyplanner_style.css" );
  root()->addWidget(d->notifications = WW<WContainerWidget>().addCss("skyplanner-notifications"));
  d->widgets = WW<WStackedWidget>( root() ).addCss("contents");
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
  telescopesPage->changed().connect([=](_n6) { d->telescopesListChanged.emit(); });
  AstroSessionsPage *astrosessionspage = new AstroSessionsPage(d->session);

  WMenuItem *mySessionsMenuItem = navBarMenu->addItem(WString::tr("mainmenu_my_sessions"), astrosessionspage);
  d->loggedInItems.push_back(mySessionsMenuItem);
  mySessionsMenuItem->setPathComponent("sessions/");


  WMenuItem *userSubMenu = WW<WMenuItem>(navBarMenu->addItem("")).addCss("hidden-xs");
  userSubMenu->setInternalPathEnabled(false);
  d->loggedInItems.push_back(userSubMenu);
  WPopupMenu *userPopup = new WPopupMenu(d->widgets);
  userPopup->setInternalPathEnabled("/");
  userSubMenu->setMenu(userPopup);

  WMenuItem *telescopesMenuItem = WW<WMenuItem>(userPopup->addItem(WString::tr("mainmenu_my_telescopes"), telescopesPage));
  d->loggedInItems.push_back(telescopesMenuItem);
  telescopesMenuItem->setPathComponent("telescopes/");

  WMenuItem *userSettingsMenuItem = WW<WMenuItem>(userPopup->addItem(WString::tr("mainmenu_my_settings"), new UserSettingsPage(d->session)));
  d->loggedInItems.push_back(userSettingsMenuItem);
  userSettingsMenuItem->setPathComponent("settings/");

  WMenuItem *logout = WW<WMenuItem>(userPopup->addItem(WString::tr("mainmenu_logout")));
  logout->setPathComponent("logout/");
  d->loggedInItems.push_back(logout);
  logout->triggered().connect([=](WMenuItem*,_n5){ d->session.login().logout(); });
  
 
  WMenuItem *feedbackMenuItem = navBarMenu->addItem(WString::tr("mainmenu_feedback"), new SendFeedbackPage(d->session));
  d->loggedInItems.push_back(feedbackMenuItem);
  feedbackMenuItem->setPathComponent("feedback/");


  auto rightMenu = new Wt::WMenu();
  navBar->addMenu(rightMenu, Wt::AlignRight);


  WMenuItem *gulinuxMenuItem = WW<WMenuItem>(rightMenu->addItem("GuLinux")).addCss("bold").addCss("menu-item-highlight");
  gulinuxMenuItem->setInternalPathEnabled(false);
  WPopupMenu *gulinuxPopup = new WPopupMenu;
  gulinuxMenuItem->setMenu(gulinuxPopup);

  auto blogMenuItem = gulinuxPopup->addItem("Blog");
  blogMenuItem->setLink("http://blog.gulinux.net");
  blogMenuItem->setLinkTarget(Wt::TargetNewWindow);
  auto skyPlannerMenuItem = gulinuxPopup->addItem("SkyPlanner Homepage");
  skyPlannerMenuItem->setLink((format("http://blog.gulinux.net/skyplanner?lang=%s") % locale().name()).str() );
  skyPlannerMenuItem->setLinkTarget(Wt::TargetNewWindow);
  
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
      userSubMenu->setText(d->loginname);
    }
    spLog("notice") << "***** "
                        << (d->session.login().loggedIn() ? "LOGIN"  : "LOGOUT")
                        << ": user " << d->loginname << " logged "
                        << (d->session.login().loggedIn() ?   "in"  : "out")
                        << "; wApp->sessionId " << sessionId();
  };
 
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
    spLog("notice") << "newPath=" << newPath;
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
  WContainerWidget *searchByNameWidget = WW<WContainerWidget>();
  d->widgets->addWidget(searchByNameWidget);
  WLineEdit *searchByNameEdit = new WLineEdit;
  navBar->addSearch(searchByNameEdit, AlignRight);
  searchByNameEdit->setTextSize(0);
  searchByNameEdit->setEmptyText(WString::tr("select_objects_widget_add_by_name"));
  auto startSearch = [=] {
    spLog("notice") << "Search by name: original=" << searchByNameEdit->valueText();
    string nameToSearch = boost::algorithm::trim_copy(searchByNameEdit->valueText().toUTF8());
    searchByNameWidget->clear();
    
    spLog("notice") << "Search by name: original=" << searchByNameEdit->valueText() << ", trimmed: " << nameToSearch << ";";
    boost::replace_all(nameToSearch, "*", "%");
    if(nameToSearch.empty()) {
      return;
    }
    
    AstroObjectsTable *resultsTable = new AstroObjectsTable(d->session, {{"buttons_add", [=](const AstroObjectsTable::Row &r){
      if(!d->session.user()) {
        WMessageBox *msg = new WMessageBox(WString::tr("search_by_name_add_to_session_caption"), WString::tr("search_by_name_no_user_text"), Information, Ok);
        msg->button(Ok)->clicked().connect([=](WMouseEvent){msg->accept();});
        msg->show();
        return;
      }
      Dbo::Transaction t(d->session);
      auto sessions = d->session.find<AstroSession>().where("user_id = ?").bind(d->session.user().id()).where("\"when\" > now()").resultList();
      if(sessions.size() == 0) {
        WMessageBox *msg = new WMessageBox(WString::tr("search_by_name_add_to_session_caption"), WString::tr("search_by_name_no_sessions_text"), Information, Ok);
        msg->button(Ok)->clicked().connect([=](WMouseEvent){msg->accept();});
        msg->show();
        return;
      }
      WDialog *addToSessionDialog = new WDialog(WString::tr("search_by_name_add_to_session_caption"));
      WComboBox *sessionsCombo = WW<WComboBox>(addToSessionDialog->contents());

      WStandardItemModel *model = new WStandardItemModel(sessionsCombo);
      for(auto session: sessions) {
        auto row = new WStandardItem(WString::fromUTF8(session->name()));
        row->setData(session);
        model->appendRow(row);
      } 
      sessionsCombo->setModel(model);
      addToSessionDialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Cancel")).css("btn-sm").onClick([=](WMouseEvent){addToSessionDialog->reject();}));
      addToSessionDialog->footer()->addWidget(WW<WPushButton>(WString::tr("buttons_add")).css("btn-primary btn-sm").onClick([=](WMouseEvent){
        addToSessionDialog->accept();
        AstroSessionPtr astroSession =  boost::any_cast<AstroSessionPtr>(model->item(sessionsCombo->currentIndex())->data());
        AstroSessionTab::add(r.astroObject.object, astroSession,  d->session, r.tableRow); 
      }));
      addToSessionDialog->show();
    }}}, false, NgcObject::allNebulaTypes(),
      {AstroObjectsTable::Names, AstroObjectsTable::Type, AstroObjectsTable::AR, AstroObjectsTable::DEC, AstroObjectsTable::Constellation, AstroObjectsTable::AngularSize, AstroObjectsTable::Magnitude}
    );
    searchByNameWidget->addWidget(resultsTable);
    
    if(d->searchByName(nameToSearch, resultsTable))
      d->widgets->setCurrentWidget(searchByNameWidget);
  };

  searchByNameEdit->changed().connect([=](_n1){ startSearch(); });
  searchByNameEdit->keyWentUp().connect([=](WKeyEvent e) { if(e.key() == Key_Enter ) startSearch(); });
  if(!d->session.login().loggedIn() && ! internalPathMatches("/dss") ) {
    setInternalPath(HOME_PATH, true);
  }
  handlePath(internalPath());
}

bool SkyPlanner::Private::searchByName(const string &name, AstroObjectsTable *table, int page)
{
  Dbo::Transaction t(session);
  int count = session.query<int>(R"(select count(distinct o.id) from "objects" o inner join denominations d on o.id = d.objects_id where lower(d.name) like '%' || ? || '%')")
    .bind(name);
  spLog("notice") << "search by name: count=" << count;
  auto tablePage = AstroObjectsTable::Page::fromCount(page, count, [=](int p) { searchByName(name, table, p); });
  if(tablePage.total > 200) {
    q->notification(WString::tr("select_objects_widget_add_by_name"), WString::tr("select_objects_widget_add_by_name_too_many"), SkyPlanner::Notification::Information, 5);
    return false;
  }

  auto ngcObjects = session.query<NgcObjectPtr>(R"(select o from "objects" o inner join denominations d on o.id = d.objects_id where lower(d.name) like '%' || ? || '%' group by o.id)")
    .bind(name).limit(tablePage.pageSize).offset(tablePage.pageSize * page).resultList();
  vector<AstroObjectsTable::AstroObject> objects;
  transform(ngcObjects.begin(), ngcObjects.end(), back_inserter(objects), [=,&t](const NgcObjectPtr &o) {
    return AstroObjectsTable::AstroObject{{}, o};
  } );
  table->populate(objects, TelescopePtr(), {}, tablePage);
  return true;
}

void SkyPlanner::Private::loadDSSPage( const std::string &hexId )
{
  WWidget *currentWidget = widgets->currentWidget();
  dssContainer->clear();
  auto objectId = Utils::fromHexString<Dbo::dbo_traits<NgcObject>::IdType>(hexId);
  Dbo::Transaction t(session);
  NgcObjectPtr ngcObject = session.find<NgcObject>().where("id = ?").bind(objectId);
  string previousPath = previousInternalPath;
  DSSPage *dssPage = new DSSPage(ngcObject, session, DSSPage::Options::standalone([=]{
    widgets->setCurrentWidget( currentWidget );
    wApp->setInternalPath( previousPath, true );
    dssContainer->clear();
  }));
  dssContainer->addWidget( dssPage );
  widgets->setCurrentWidget( dssContainer );
}

WLogEntry SkyPlanner::uLog(const string &type) const
{
    return WApplication::log(type) << "{user: '" << (d->loginname.empty() ? "anonymous" : WString("{1} ({2})").arg(d->loginname).arg(static_cast<long>(d->session.user().id())) ) << "'} ";
}


shared_ptr<SkyPlanner::Notification> SkyPlanner::notification(const WString &title, const WString &content, Notification::Type type, int autoHideSeconds, WContainerWidget *addTo)
{
  auto notification = make_shared<Notification>(title, content, type, true);
  (addTo ? addTo : d->notifications)->addWidget(notification->widget() );
  notification->widget()->animateShow({WAnimation::Fade, WAnimation::EaseInOut, 500});
  if(autoHideSeconds > 0)
    WTimer::singleShot(1000*autoHideSeconds, [=](WMouseEvent) { notification->close(); } );
  d->shownNotifications.insert(notification);
  notification->closed().connect([=](_n6) { d->shownNotifications.erase(notification); });
  return notification;
}

class SkyPlanner::Notification::Private {
public:
  Signal<> closed;
  WContainerWidget *widget;
  bool valid = true;
};

Signal<> &SkyPlanner::Notification::closed() const
{
  return d->closed;
}

WWidget *SkyPlanner::Notification::widget() const
{
  return d->widget;
}

void SkyPlanner::Notification::close()
{
  spLog("notice") << " valid=" << valid();
  if(!valid())
    return;
  d->valid = false;
  d->closed.emit();
  d->widget->hide();
  delete d->widget;
//  WTimer::singleShot(3000, [=](WMouseEvent){delete d->widget; d->widget = nullptr; });
}

bool SkyPlanner::Notification::valid() const
{
  return d->valid;
}

SkyPlanner::Notification::Notification(const WString &title, const WString &content, Type type, bool addCloseButton, WContainerWidget *parent)
  : d()
{
  static map<Type,string> notificationStyles {
    {Error, "alert-danger"},
    {Success, "alert-success"},
    {Information, "alert-info"},
    {Alert, "alert-warning"},
  };
  d->widget = WW<WContainerWidget>().addCss("alert").addCss("alert-block").addCss(notificationStyles[type]);
  if(addCloseButton) {
    WPushButton *closeButton = WW<WPushButton>().css("close").onClick([=](WMouseEvent) { close(); } );
    closeButton->setTextFormat(XHTMLUnsafeText);
    closeButton->setText("<h4><strong>&times;</strong></h4>");
    d->widget->addWidget(closeButton);
  }

  d->widget->addWidget(new WText{WString("<h4>{1}</h4>").arg(title) });
  d->widget->addWidget(new WText{content});
}

SkyPlanner::Notification::~Notification()
{

}


Signal<> &SkyPlanner::telescopesListChanged() const
{
  return d->telescopesListChanged;
}
