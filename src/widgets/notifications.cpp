/*
 * Copyright (C) 2016  Marco Gulino <marco.gulino@gmail.com>
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

#include "notifications.h"
#include "wt_helpers.h"
#include <Wt/WText>
#include <Wt/WTimer>
#include <set>
using namespace std;
using namespace Wt;
using namespace WtCommons;

class Notifications::Private {
public:
    Private(Notifications *q);
    set<Notification::ptr> shownNotifications;

private:
    Notifications *q;
};

Notifications::Private::Private(Notifications* q) : q(q)
{
}

Notifications::~Notifications()
{
}

Notifications::Notifications(Wt::WContainerWidget* parent)
    : dptr(this)
{
  addStyleClass("skyplanner-notifications");
  addStyleClass("hidden-print");
}

void Notifications::clearNotifications()
{
  for(auto notification: d->shownNotifications)
    notification->close();
  d->shownNotifications.clear();
}

Notification::ptr Notifications::show(const Wt::WString& title, Wt::WWidget* content, Notification::Type type, int autoHideSeconds, Wt::WContainerWidget* addTo, const std::__cxx11::string& categoryTag)
{
  auto notification = make_shared<Notification>(title, content, type, true, categoryTag);
  (addTo ? addTo : this)->addWidget(notification->widget() );
  notification->widget()->animateShow({WAnimation::Fade, WAnimation::EaseInOut, 500});
  if(!categoryTag.empty()) {
    auto old_notification = find_if(begin(d->shownNotifications), end(d->shownNotifications), [categoryTag](const shared_ptr<Notification> &n){ return n->categoryTag() == categoryTag; });
    if(old_notification != end(d->shownNotifications)) {
      (*old_notification)->close();
    }
  }
  if(autoHideSeconds > 0)
    WTimer::singleShot(1000*autoHideSeconds, [=](WMouseEvent) { notification->close(); } );
  d->shownNotifications.insert(notification);
  notification->closed().connect([=](_n6) { d->shownNotifications.erase(notification); });
  return notification;
}

Notification::ptr Notifications::show(const Wt::WString& title, const Wt::WString& content, Notification::Type type, int autoHideSeconds, Wt::WContainerWidget* addTo, const std::__cxx11::string& categoryTag)
{
  return show(title, new WText(content), type, autoHideSeconds, addTo, categoryTag);
}
