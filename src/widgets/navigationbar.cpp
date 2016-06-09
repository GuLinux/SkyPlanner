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

    
    struct LoadedMenu {
      NavigationBar::MenuItem menu_item;
      WMenuItem *wmenu_item;
    };
    typedef list<LoadedMenu> LoadedMenus;
    LoadedMenus loaded_menus;
    void load(const NavigationBar::MenuItem &menu_item, WMenu *menu);
    void update_visibility();
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
    : WNavigationBar(parent), dptr(session, this)
{
  addStyleClass("navbar-inverse");
  setResponsive(true);
  setTitle( WString::tr("application_title"), WLink(WLink::InternalPath, URLs::home) );
  WMenu *navBarMenu = new WMenu(stack);
  navBarMenu->setInternalPathEnabled("/");
  addMenu(navBarMenu);
  GuLinux::make_stream(menu_items).for_each([=](const auto &item) {d->load(item, navBarMenu); });
  session.login().changed().connect(bind(&Private::update_visibility, d.get()));
  d->update_visibility();
}

void NavigationBar::Private::update_visibility()
{
  bool logged_in_hidden = !session.login().loggedIn();
  bool logged_out_hidden = !logged_in_hidden;
  auto items = GuLinux::make_stream_copy(loaded_menus)
    .remove([](const LoadedMenu &item){ return item.menu_item.visibility == NavigationBar::MenuItem::Both; })
    .for_each([=](const LoadedMenu &item){ item.wmenu_item->setHidden( item.menu_item.visibility == NavigationBar::MenuItem::LoggedIn ? logged_in_hidden : logged_out_hidden ); });
}


void NavigationBar::Private::load(const NavigationBar::MenuItem& menu_item, WMenu *menu)
{
  auto item = menu->addItem(menu_item.key.empty() ? WString{} : WString::tr(menu_item.key), menu_item.widget);
  loaded_menus.push_back({menu_item, item});
  for(auto style: menu_item.css)
    item->addStyleClass(style);
  if(menu_item.path.empty())
    item->setInternalPathEnabled(false);
  else
    item->setPathComponent(menu_item.path);
  if(!menu_item.children.empty()) {
    auto popup = new WPopupMenu(menu->contentsStack());
    if(menu->internalPathEnabled())
      popup->setInternalPathEnabled(menu->internalBasePath());
    for(auto child: menu_item.children)
      load(child,popup);
    item->setMenu(popup);
  }
  if(menu_item.ptr)
    *menu_item.ptr = item;
}

