#ifndef FILTERBYOBSERVEDWIDGET_P_H
#define FILTERBYOBSERVEDWIDGET_P_H

#include "widgets/filterbyobservedwidget.h"
#include <Wt/WComboBox>

class FilterByObservedWidget::Private
{
public:
  Private(FilterByObservedWidget *q);
  Wt::Signal<> changed;
  Wt::WComboBox *combo;
private:
  FilterByObservedWidget *q;
};

#endif // FILTERBYOBSERVEDWIDGET_H


