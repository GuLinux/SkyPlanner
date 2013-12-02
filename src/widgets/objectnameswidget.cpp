/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#include "Models"
#include "objectnameswidget.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/format.h"
#include <Wt/WText>
#include <Wt/Utils>
#include "utils/d_ptr_implementation.h"
#include "utils/autopostresource.h"
#include <Wt/WPopupMenu>
#include <Wt/WAnchor>

using namespace WtCommons;
using namespace Wt;
using namespace std;

class ObjectNamesWidget::Private {
public:
  Private(ObjectNamesWidget *q) : q(q) {}
private:
  ObjectNamesWidget * const q;
};

ObjectNamesWidget::ObjectNamesWidget(const Wt::Dbo::ptr<NgcObject> &object, WContainerWidget *parent) : WContainerWidget(parent), d(this)
{
    auto names = object->nebulae();
    stringstream namesStream;
    string separator;
    for(auto name: names) {
      namesStream << separator << name->name();
      separator = ", ";
    }
    WAnchor *namesText = WW<WAnchor>("", Utils::htmlEncode(WString::fromUTF8(namesStream.str()))).css("link");
    namesText->clicked().connect([=](WMouseEvent e) {
      WPopupMenu *popup = new WPopupMenu();
      popup->addSectionHeader("More Information");
      WMenuItem *ngcIcMenuItem = popup->addItem("NGC-IC Project Page");
      ngcIcMenuItem->setLink(new AutoPostResource{"http://www.ngcicproject.org/ngcicdb.asp", {{"ngcicobject", object->objectId() }}, popup});
      ngcIcMenuItem->setLinkTarget(TargetNewWindow);
      popup->popup(e);
    });
    addWidget(namesText);
}

ObjectNamesWidget::~ObjectNamesWidget()
{
}