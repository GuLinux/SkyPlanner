#ifndef FILTERBYMAGNITUDEWIDGET_P_H
#define FILTERBYMAGNITUDEWIDGET_P_H

#include "widgets/filterbymagnitudewidget.h"
namespace Wt {
  class WSlider;
}
class FilterByMagnitudeWidget::Private
{
public:
  Private(Range range, FilterByMagnitudeWidget *q);
  Wt::Signal<> changed;
  Wt::WSlider *magnitudeSlider;
  Wt::WPushButton *button;
  Range range;
  Range initialRange;
  void updateLabel();
private:
  FilterByMagnitudeWidget *q;
};


#endif // FILTERBYMAGNITUDEWIDGET_H


