#ifndef FILTERBYALTITUDEWIDGET_P_H
#define FILTERBYALTITUDEWIDGET_P_H

#include "widgets/filterbyaltitudewidget.h"
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WComboBox>
class FilterByAltitudeWidget::Private
{
public:
  Private(const Angle &initialValue, FilterByAltitudeWidget *q);
  Angle initialValue;
  Wt::WStandardItemModel *altitudeModel;
  Wt::WComboBox *altitudeCombo;
  Wt::Signal<> changed;
private:
  FilterByAltitudeWidget *q;
};

#endif // FILTERBYALTITUDEWIDGET_H


