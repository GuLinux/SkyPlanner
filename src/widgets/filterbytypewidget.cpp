/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
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
#include "filterbytypewidget.h"
#include "private/filterbytypewidget_p.h"

#include <Wt/WCheckBox>
#include "wt_helpers.h"
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WCheckBox>
#include <Wt/WToolBar>

using namespace std;
using namespace Wt;
using namespace WtCommons;

FilterByTypeWidget::Private::Private(const set<NgcObject::NebulaType> &initialSelection, FilterByTypeWidget *q): nebulaTypeFilters(initialSelection), initialSelection(initialSelection), q(q)
{
}

FilterByTypeWidget::FilterByTypeWidget(const set<NgcObject::NebulaType> &initialSelection, WContainerWidget *parent)
  : WCompositeWidget(parent), dptr(initialSelection, this)
{
  WPushButton *astroTypeButton = WW<WPushButton>(WString::tr("filter_by_type_button")).css("btn-sm btn-link filter-widget-link");
  astroTypeButton->clicked().connect([=](const WMouseEvent &e) {
    set<NgcObject::NebulaType> initialSelection = d->nebulaTypeFilters;
    WDialog *dialog = new WDialog(WString::tr("filter_by_type_title"));
    map<NgcObject::NebulaType, WCheckBox*> items;
    for(auto type: NgcObject::allNebulaTypes()) {
      items[type] = WW<WCheckBox>(NgcObject::typeDescription(type));
      items[type]->setChecked(initialSelection.count(type)>0);
    }

    dialog->contents()->addWidget(WW<WToolBar>()
      .addButton(WW<WPushButton>(WString::tr("ngcobject_type_All")).onClick([=](WMouseEvent) { for(auto item: items) item.second->setChecked();  } ) )
      .addButton(WW<WPushButton>(WString::tr("ngcobject_type_AllButStars")).onClick([=](WMouseEvent){ for(auto item: items) item.second->setChecked( NgcObject::allNebulaTypesButStars().count(item.first)>0 ); }) )
      .addButton(WW<WPushButton>(WString::tr("ngcobject_type_None")).onClick([=](WMouseEvent) { for(auto item: items) item.second->setUnChecked();  } ) )
    );
    for(auto type: NgcObject::allNebulaTypes()) {
      dialog->contents()->addWidget(WW<WContainerWidget>().css("checkbox").add(items[type]));
    }

    dialog->setClosable(true);
    dialog->finished().connect([=](WDialog::DialogCode result, _n5) {
      if(result != WDialog::Accepted) return;
      d->nebulaTypeFilters.clear();
      for(auto item: items)
        if(item.second->isChecked())
          d->nebulaTypeFilters.insert(item.first);
      if(d->nebulaTypeFilters != initialSelection)
        d->changed.emit();
    });
    dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Cancel")).addCss("btn-danger").onClick([=](WMouseEvent) { dialog->reject(); }));
    dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).addCss("btn-primary").onClick([=](WMouseEvent) { dialog->accept(); }));

    dialog->show();
  });
  setImplementation(astroTypeButton);
}

FilterByTypeWidget::~FilterByTypeWidget()
{
}

Wt::Signal<> &FilterByTypeWidget::changed() const
{
  return d->changed;
}

set<NgcObject::NebulaType> FilterByTypeWidget::selected() const
{
  return d->nebulaTypeFilters;
}

void FilterByTypeWidget::resetDefaultValue()
{
  d->nebulaTypeFilters = d->initialSelection;
  d->changed.emit();
}
