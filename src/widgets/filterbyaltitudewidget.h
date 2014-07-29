#ifndef FILTERBYALTITUDEWIDGET_H
#define FILTERBYALTITUDEWIDGET_H

#include <utils/d_ptr.h>
#include "types.h"
#include <Wt/WCompositeWidget>

class FilterByAltitudeWidget : public Wt::WCompositeWidget {
public:
  explicit FilterByAltitudeWidget(const Wt::WString &labelText, const Angle &initial, const Angle &steps = Angle::degrees(10));
  ~FilterByAltitudeWidget();
  Wt::Signal<> &changed() const;
  void resetDefaultValue();
  Angle currentValue() const;
private:
  D_PTR;
};

#endif // FILTERBYALTITUDEWIDGET_H


