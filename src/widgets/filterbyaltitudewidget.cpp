#include "filterbyaltitudewidget.h"
#include "private/filterbyaltitudewidget_p.h"

#include <utils/d_ptr_implementation.h>

#include "Wt-Commons/wt_helpers.h"
#include "utils/format.h"
#include <Wt/WLabel>


using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByAltitudeWidget::Private::Private(const Angle &initialValue, FilterByAltitudeWidget *q): initialValue(initialValue), q(q)
{
}

FilterByAltitudeWidget::FilterByAltitudeWidget(const WString &labelText, const Angle &initial, const Angle &steps)
  : d(initial, this)
{
  d->altitudeModel = new WStandardItemModel(this);
  d->altitudeCombo = WW<WComboBox>().css("input-sm");
  d->altitudeCombo->setModel(d->altitudeModel);
  for(Angle i=Angle::degrees(0); i<Angle::degrees(90); i+=steps ) {
    auto item = new WStandardItem(format("%d°") % i.degrees());
    item->setData(i);
    d->altitudeModel->appendRow(item);
  }
  d->altitudeCombo->activated().connect([=](int,_n5){ d->changed.emit(); });
  setImplementation(WW<WContainerWidget>().setInline(true).add(new WLabel{labelText}).add(d->altitudeCombo));
}

FilterByAltitudeWidget::~FilterByAltitudeWidget()
{
}

Signal<> &FilterByAltitudeWidget::changed() const
{
  return d->changed;
}

void FilterByAltitudeWidget::resetDefaultValue()
{
  for(int i=0; i<d->altitudeModel->rowCount(); i++) {
    if(d->initialValue == boost::any_cast<Angle>(d->altitudeModel->item(i)->data())) {
      d->altitudeCombo->setCurrentIndex(i);
    }
  }
  d->changed.emit();
}

Angle FilterByAltitudeWidget::currentValue() const
{
  return boost::any_cast<Angle>(d->altitudeModel->item(d->altitudeCombo->currentIndex() )->data());
}
