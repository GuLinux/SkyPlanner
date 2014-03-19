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

FilterByTypeWidget::Private::Private(FilterByTypeWidget *q): q(q)
{
}

FilterByTypeWidget::FilterByTypeWidget(WContainerWidget *parent)
  : WCompositeWidget(parent), d(this)
{
  WPushButton *astroTypeButton = WW<WPushButton>(WString::tr("object_column_type"));
  WPopupMenu *astroTypeMenu = new WPopupMenu;
  astroTypeButton->setMenu(astroTypeMenu);


  astroTypeMenu->addSeparator();
  map<NgcObject::NebulaType, WMenuItem*> menuItems;
  for(auto type: NgcObject::nebulaTypes())
    menuItems[type] = astroTypeMenu->addItem(NgcObject::typeDescription(type));

  auto selectFilters = [=] (function<bool(NgcObject::NebulaType)> includeFunc) {
    d->nebulaTypeFilters.clear();
    for(auto type: NgcObject::nebulaTypes())
      if(includeFunc(type)) d->nebulaTypeFilters.insert(type);
    for(auto item: menuItems)
      item.second->checkBox()->setChecked(d->nebulaTypeFilters.count(item.first));
  };

  for(auto item: menuItems) {
     item.second->setCheckable(true);
     item.second->checkBox()->changed().connect([=](_n1){
       selectFilters([=](NgcObject::NebulaType t) { return menuItems.at(t)->isChecked(); });
       d->changed.emit();
     });
  }

  auto selectAllButStars = [](NgcObject::NebulaType t) { return t != NgcObject::RedStar && t != NgcObject::Asterism; };

  astroTypeMenu->insertItem(0, NgcObject::typeDescription(NgcObject::AllButStars))->triggered().connect([=](WMenuItem*, _n5){
    selectFilters(selectAllButStars);
    d->changed.emit();
  });
  astroTypeMenu->insertItem(1, NgcObject::typeDescription(NgcObject::All))->triggered().connect([=](WMenuItem*, _n5){
    d->nebulaTypeFilters.clear();
    selectFilters([](NgcObject::NebulaType) { return true; });
    d->changed.emit();
  });

  selectFilters(selectAllButStars);
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
