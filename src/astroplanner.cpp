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

using namespace std;
using namespace Wt;
using namespace WtCommons;

AstroPlanner::Private::Private( AstroPlanner *q ) : q( q )
{
}

AstroPlanner::~AstroPlanner()
{
}

AstroPlanner::AstroPlanner( const WEnvironment &environment )
  : WApplication( environment ), d( this )
{
  setTheme( new WBootstrapTheme( this ) );
  cerr << __PRETTY_FUNCTION__ << endl;
  WNavigationBar *navBar = WW<WNavigationBar>( root() ).addCss( "navbar-inverse" );
  navBar->setResponsive( true );
  navBar->setTitle( "AstroPlanner" );
  useStyleSheet( "/style.css" );
  WStackedWidget *widgets = new WStackedWidget( root() );
  widgets->setTransitionAnimation({WAnimation::AnimationEffect::Fade});
  WMenu *navBarMenu = new WMenu(widgets);
  navBar->addMenu(navBarMenu);
  Auth::AuthWidget *authWidget = new Auth::AuthWidget( Session::auth(), d->session.users(), d->session.login() );
  authWidget->model()->addPasswordAuth( &Session::passwordAuth() );
  authWidget->setRegistrationEnabled( true );
  authWidget->processEnvironment();
  
  WMenuItem *authMenuItem;
  d->loggedOutItems.push_back(authMenuItem = navBarMenu->addItem("Login", authWidget));
  TelescopesPage *telescopesPage = new TelescopesPage(d->session);
  AstroSessionsPage *astrosessionspage = new AstroSessionsPage(d->session);
  
  WMenuItem *mySessionsMenuItem;
  
  d->loggedInItems.push_back(mySessionsMenuItem = navBarMenu->addItem("My Sessions", astrosessionspage));
  d->loggedInItems.push_back(navBarMenu->addItem("My Telescopes", telescopesPage));
  WMenuItem *logout = navBarMenu->addItem("Logout");
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
}
