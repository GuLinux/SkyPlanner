#ifndef FILTERBYALTITUDEWIDGET_H
#define FILTERBYALTITUDEWIDGET_H

#include <utils/d_ptr.h>
#include "types.h"
#include <Wt/WCompositeWidget>

class FilterByAltitudeWidget : public Wt::WCompositeWidget {
public:
  struct AvailableAngles {
    AvailableAngles(const Angle &initial, const Angle &start, const Angle &end, const Angle &steps = Angle::degrees(10))
      : initial(initial), start(start), end(end), steps(steps) {}
    Angle initial;
    Angle start;
    Angle end;
    Angle steps;
  };

  explicit FilterByAltitudeWidget(const Wt::WString &labelText, const AvailableAngles &availableAngles, Wt::WContainerWidget *parent = 0);
  ~FilterByAltitudeWidget();
  Wt::Signal<> &changed() const;
  void resetDefaultValue();
  Angle currentValue() const;
private:
  D_PTR;
};

#endif // FILTERBYALTITUDEWIDGET_H


