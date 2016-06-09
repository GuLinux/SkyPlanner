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

#include "notification.h"
#include "wt_helpers.h"
#include "skyplanner.h"
#include <Wt/WPushButton>
using namespace std;
using namespace Wt;
using namespace WtCommons;


class Notification::Private {
public:
  Signal<> closed;
  WContainerWidget *widget;
  bool valid = true;
  string categoryTag;
};

Signal<> &Notification::closed() const
{
  return d->closed;
}

WWidget *Notification::widget() const
{
  return d->widget;
}

void Notification::close()
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

bool Notification::valid() const
{
  return d->valid;
}

Notification::Notification(const WString& title, WWidget* content, Notification::Type type, bool addCloseButton, const string& categoryTag, WContainerWidget* parent)
  : dptr()
{
  d->categoryTag = categoryTag;
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
  d->widget->addWidget(content);
}

string Notification::categoryTag() const
{
  return d->categoryTag;
}


Notification::~Notification()
{
}