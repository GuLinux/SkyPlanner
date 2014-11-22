#ifndef FILTERBYMAGNITUDEWIDGET_P_H
#define FILTERBYMAGNITUDEWIDGET_P_H

#include "widgets/filterbymagnitudewidget.h"
namespace Wt {
  class WSlider;
}
class FilterByMagnitudeWidget::Private
{
public:
  Private(double initialValue, FilterByMagnitudeWidget *q);
  Wt::Signal<> changed;
  Wt::WString minimumValueText;
  Wt::WString maximumValueText;
  Wt::WSlider *magnitudeSlider;
  Wt::WText *valueLabel;
  double initialValue;
  void checkValue();
  Wt::WPushButton *button;
  void setLabel(Range range);
private:
  FilterByMagnitudeWidget *q;
};

#endif // FILTERBYMAGNITUDEWIDGET_H


