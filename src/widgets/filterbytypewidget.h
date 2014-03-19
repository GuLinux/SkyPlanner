#ifndef FILTERBYTYPEWIDGET_H
#define FILTERBYTYPEWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <set>
#include "models/ngcobject.h"

class FilterByTypeWidget : public Wt::WCompositeWidget {
public:
  explicit FilterByTypeWidget(const std::set<NgcObject::NebulaType> &initialSelection, Wt::WContainerWidget *parent = 0);
  ~FilterByTypeWidget();
  Wt::Signal<> &changed() const;
  std::set<NgcObject::NebulaType> selected() const;
private:
  D_PTR;
};

#endif // FILTERBYTYPEWIDGET_H


