#include "filterbymagnitudewidget.h"
#include "private/filterbymagnitudewidget_p.h"

#include <utils/d_ptr_implementation.h>
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WSlider>
#include <Wt/WText>
#include "utils/format.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByMagnitudeWidget::Private::Private(FilterByMagnitudeWidget *q): q(q)
{
}

FilterByMagnitudeWidget::FilterByMagnitudeWidget(const Options &options, const Range &range, WContainerWidget *parent)
 : FilterByMagnitudeWidget(options, range, range.minimum, parent)
{
}

FilterByMagnitudeWidget::FilterByMagnitudeWidget(const Options &options, const Range &range, double initialValue, WContainerWidget *parent)
  : WCompositeWidget(parent), d(this)
{
  WText *valueLabel = WW<WText>().css("badge");
  d->magnitudeSlider = WW<WSlider>().css("form-slider");
  d->magnitudeSlider->setNativeControl(false);
  setRange(range);
  d->minimumValueText = options.minimumValueText;
  d->maximumValueText = options.maximumValueText;


  auto checkValue = [=] {
    valueLabel->setText(format("%.1f") % magnitude() );
    if(magnitude() == d->magnitudeSlider->minimum() && !d->minimumValueText.empty())
      valueLabel->setText(d->minimumValueText);
    if(magnitude() == d->magnitudeSlider->maximum() && !d->maximumValueText.empty())
      valueLabel->setText(d->minimumValueText);
  };
  d->magnitudeSlider->setValue(initialValue*10.);
  checkValue();

  d->magnitudeSlider->valueChanged().connect([=](int value, _n5){
    checkValue();
    d->changed.emit(magnitude());
  });
#ifndef PRODUCTION_MODE
#warning Logging magnitudeSlider sliderMoved
  d->magnitudeSlider->sliderMoved().connect("function(a, b) { console.log(a); console.log(b); var lastSliderMoved = b; }");
#endif
  setImplementation(WW<WContainerWidget>().setInline(true).add(new WText{options.labelText}).add(d->magnitudeSlider).add(valueLabel));
}

FilterByMagnitudeWidget::~FilterByMagnitudeWidget()
{
}

Signal<double> &FilterByMagnitudeWidget::changed() const
{
  return d->changed;
}

void FilterByMagnitudeWidget::setMinimum(double minimum)
{
  d->magnitudeSlider->setMinimum(minimum*10);
}

void FilterByMagnitudeWidget::setMaximum(double maximum)
{
  d->magnitudeSlider->setMaximum(maximum*10);
}

void FilterByMagnitudeWidget::setRange(const Range &range)
{
  setMinimum(range.minimum);
  setMaximum(range.maximum);
}

double FilterByMagnitudeWidget::magnitude() const
{
  return static_cast<double>(d->magnitudeSlider->value()) / 10.;
}

bool FilterByMagnitudeWidget::isMinimum() const
{
  return d->magnitudeSlider->value() == d->magnitudeSlider->minimum();
}

bool FilterByMagnitudeWidget::isMaximum() const
{
  return d->magnitudeSlider->value() == d->magnitudeSlider->maximum();
}



