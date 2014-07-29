#ifndef FILTERBYTYPEWIDGET_P_H
#define FILTERBYTYPEWIDGET_P_H

#include "widgets/filterbytypewidget.h"
#include "models/Models"

class FilterByTypeWidget::Private
{
public:
  Private(const std::set<NgcObject::NebulaType> &initialSelection, FilterByTypeWidget *q);
  Wt::Signal<> changed;
  std::set<NgcObject::NebulaType> nebulaTypeFilters;
  std::set<NgcObject::NebulaType> initialSelection;

private:
  FilterByTypeWidget *q;
};

#endif // FILTERBYTYPEWIDGET_H


