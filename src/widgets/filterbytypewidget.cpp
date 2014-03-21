#include "filterbytypewidget.h"
#include "private/filterbytypewidget_p.h"

#include <utils/d_ptr_implementation.h>
#include <Wt/WCheckBox>
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WPopupMenu>
#include <Wt/WPushButton>

using namespace std;
using namespace Wt;
using namespace WtCommons;

FilterByTypeWidget::Private::Private(const set<NgcObject::NebulaType> &initialSelection, FilterByTypeWidget *q): nebulaTypeFilters(initialSelection), q(q)
{
}

FilterByTypeWidget::FilterByTypeWidget(const set<NgcObject::NebulaType> &initialSelection, WContainerWidget *parent)
  : WCompositeWidget(parent), d(initialSelection, this)
{
  WPushButton *astroTypeButton = WW<WPushButton>(WString::tr("object_column_type"));
  WPopupMenu *astroTypeMenu = new WPopupMenu;
  astroTypeButton->setMenu(astroTypeMenu);


  astroTypeMenu->addSeparator();
  map<NgcObject::NebulaType, WMenuItem*> menuItems;
  for(auto type: NgcObject::allNebulaTypes())
    menuItems[type] = astroTypeMenu->addItem(NgcObject::typeDescription(type));

  auto syncFilters = [=] {
    for(auto item: menuItems)
      item.second->checkBox()->setChecked(d->nebulaTypeFilters.count(item.first));
  };

  for(auto item: menuItems) {
     item.second->setCheckable(true);
     item.second->checkBox()->changed().connect([=](_n1){
       if(item.second->isChecked())
         d->nebulaTypeFilters.insert(item.first);
       else
         d->nebulaTypeFilters.erase(item.first);
       syncFilters();
       d->changed.emit();
     });
  }

  astroTypeMenu->insertItem(0, WString::tr("ngcobject_type_AllButStars"))->triggered().connect([=](WMenuItem*, _n5){
    d->nebulaTypeFilters = NgcObject::allNebulaTypesButStars();
    syncFilters();
    d->changed.emit();
  });
  astroTypeMenu->insertItem(1, WString::tr("ngcobject_type_All"))->triggered().connect([=](WMenuItem*, _n5){
    d->nebulaTypeFilters = NgcObject::allNebulaTypes();
    syncFilters();
    d->changed.emit();
  });
   astroTypeMenu->insertItem(2, WString::tr("ngcobject_type_None"))->triggered().connect([=](WMenuItem*, _n5){
    d->nebulaTypeFilters.clear();
    syncFilters();
    d->changed.emit();
  });
 syncFilters();
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
