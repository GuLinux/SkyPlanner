#include "filterbymagnitudewidget.h"
#include "private/filterbymagnitudewidget_p.h"

#include <utils/d_ptr_implementation.h>
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WSlider>
#include <Wt/WText>
#include "utils/format.h"
#include "skyplanner.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByMagnitudeWidget::Private::Private(double initialValue, FilterByMagnitudeWidget *q): initialValue(initialValue), q(q)
{
}

FilterByMagnitudeWidget::FilterByMagnitudeWidget(const Options &options, const Range &range, WContainerWidget *parent)
 : FilterByMagnitudeWidget(options, range, range.minimum, parent)
{
}

FilterByMagnitudeWidget::FilterByMagnitudeWidget(const Options &options, const Range &range, double initialValue, WContainerWidget *parent)
  : WCompositeWidget(parent), d(initialValue, this)
{
  d->valueLabel = WW<WText>().css("badge");
  d->magnitudeSlider = WW<WSlider>().css("form-slider magnitude-slider");
  d->magnitudeSlider->setNativeControl(false);
  setRange(range);
  d->minimumValueText = options.minimumValueText;
  d->maximumValueText = options.maximumValueText;


  d->magnitudeSlider->setValue(initialValue*10.);
  d->checkValue();

  d->magnitudeSlider->valueChanged().connect([=](int value, _n5){
    d->checkValue();
    d->changed.emit();
  });
#ifndef PRODUCTION_MODE
#warning Logging magnitudeSlider sliderMoved
  d->magnitudeSlider->sliderMoved().connect("function(a, b) { console.log(a); console.log(b); var lastSliderMoved = b; }");
#endif
  setImplementation(WW<WContainerWidget>().setInline(true).add(new WText{WString("<small>{1}</small>").arg(options.labelText)}).add(d->magnitudeSlider).add(d->valueLabel));
  d->magnitudeSlider->setInline(true);
}

void FilterByMagnitudeWidget::Private::checkValue()
{
  spLog("notice") << " magnitude: " << q->magnitude() << ", minimum: " << magnitudeSlider->minimum() << ", maximum: " << magnitudeSlider->maximum() << "minText: " << minimumValueText << ", maxText: " << maximumValueText;
  valueLabel->setText(format("%.1f") % q->magnitude() );
  if(q->magnitude()*10 == magnitudeSlider->minimum() && !minimumValueText.empty())
    valueLabel->setText(minimumValueText);
  if(q->magnitude()*10 == magnitudeSlider->maximum() && !maximumValueText.empty())
    valueLabel->setText(maximumValueText);
}

FilterByMagnitudeWidget::~FilterByMagnitudeWidget()
{
}

Signal<> &FilterByMagnitudeWidget::changed() const
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


void FilterByMagnitudeWidget::resetDefaultValue()
{
  d->magnitudeSlider->setValue(d->initialValue*10.);
  d->checkValue();
  d->changed.emit();
}
