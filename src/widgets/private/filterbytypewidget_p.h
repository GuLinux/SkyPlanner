#ifndef FILTERBYTYPEWIDGET_P_H
#define FILTERBYTYPEWIDGET_P_H

#include "widgets/filterbytypewidget.h"
#include "models/Models"

class FilterByTypeWidget::Private
{
public:
  Private(FilterByTypeWidget *q);
  Wt::Signal<> changed;
  std::set<NgcObject::NebulaType> nebulaTypeFilters;

private:
  FilterByTypeWidget *q;
};

#endif // FILTERBYTYPEWIDGET_H


