#ifndef FILTERBYMAGNITUDEWIDGET_P_H
#define FILTERBYMAGNITUDEWIDGET_P_H

#include "widgets/filterbymagnitudewidget.h"
namespace Wt {
  class WSlider;
}
class FilterByMagnitudeWidget::Private
{
public:
  Private(FilterByMagnitudeWidget *q);
  Wt::Signal<double> changed;
  Wt::WString minimumValueText;
  Wt::WString maximumValueText;
  Wt::WSlider *magnitudeSlider;
private:
  FilterByMagnitudeWidget *q;
};

#endif // FILTERBYMAGNITUDEWIDGET_H


