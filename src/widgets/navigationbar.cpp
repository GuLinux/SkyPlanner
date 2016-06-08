/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
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

#include "navigationbar.h"
#include <Wt/WMenu>
#include <Wt/WPopupMenu>
#include <Wt/Auth/Login>
#include <Wt/WStackedWidget>
#include <functional>
#include "session.h"
#include <urls.h>
#include "c++/containers_streams.h"

using namespace Wt;
using namespace std;
class NavigationBar::Private {
public:
    Private(Session &session, NavigationBar *q);
    Session& session;

private:
    NavigationBar *q;
};

NavigationBar::Private::Private(Session& session, NavigationBar* q) : session{session}, q(q)
{
}

NavigationBar::~NavigationBar()
{
}

NavigationBar::NavigationBar(const MenuItem::list &menu_items, WStackedWidget *stack, Session &session, Wt::WContainerWidget* parent)
    : dptr(session, this)
{
  addStyleClass("navbar-inverse");
  setResponsive(true);
  setTitle( WString::tr("application_title"), WLink(WLink::InternalPath, URLs::home) );
  WMenu *navBarMenu = new WMenu(stack);
  navBarMenu->setInternalPathEnabled("/");
  addMenu(navBarMenu);
  GuLinux::make_stream(menu_items).for_each([=](const auto &item) {item.addTo(navBarMenu, d->session); });
}

void NavigationBar::MenuItem::addTo(Wt::WMenu* menu, Session& session) const
{
  auto item = menu->addItem(key.empty() ? WString{} : WString::tr(key), widget);
  for(auto style: css)
    item->addStyleClass(style);
  if(path.empty())
    item->setInternalPathEnabled(false);
  else
    item->setPathComponent(path);
  if(!children.empty()) {
    auto popup = new WPopupMenu(menu->contentsStack());
    if(menu->internalPathEnabled())
      popup->setInternalPathEnabled(menu->internalBasePath());
    for(auto child: children)
      child.addTo(popup,session);
    item->setMenu(popup);
  }
  if(visibility != Both) {
    auto showhide = [=, &session]{
      bool loggedIn = session.login().loggedIn();
      bool hidden = visibility ==  LoggedIn ? !loggedIn : loggedIn;
      std::cerr << "NavBarMenuItem: loggedIn: " << loggedIn << ", widget: " << key << " with visibility " << visibility << ", setting hidden: " << hidden << std::endl;
      item->setHidden( hidden );
    };
    session.login().changed().connect(bind(showhide));
    showhide();
  }
  if(ptr)
    *ptr = item;
}
