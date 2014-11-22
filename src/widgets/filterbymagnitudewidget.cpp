#include "filterbymagnitudewidget.h"
#include "private/filterbymagnitudewidget_p.h"

#include <utils/d_ptr_implementation.h>
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WSlider>
#include <Wt/WText>
#include <Wt/WAnchor>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WLabel>
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

  setImplementation(d->button = WW<WPushButton>().addCss("btn-sm btn-link filter-widget-link"));
  d->button->clicked().connect([=](WMouseEvent){
    WDialog *dialog = new WDialog;
    dialog->setClosable(true);
    dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).onClick([=](WMouseEvent){dialog->accept();}));
    auto slider = [=](double value) {
      auto _slider = new WSlider(Horizontal);
      _slider->setWidth(400);
      _slider->setMinimum(-50);
      _slider->setMaximum(250);
      _slider->setValue(value*10);
      WText *label = WW<WText>().css("badge pull-right");
      auto set_label = [=](double value){
	label->setText(WString::fromUTF8(format("%.2f") % value));
      };
      set_label(value);
      _slider->valueChanged().connect([=](int v, _n5){ set_label(static_cast<double>(v) / 10.); });
      dialog->contents()->addWidget(label);
      dialog->contents()->addWidget(WW<WContainerWidget>().add(_slider));
      return _slider;
    };
    dialog->contents()->addWidget(WW<WLabel>(WString::tr("minimum_magnitude_label")));
    dialog->contents()->addWidget(slider(-5));
    dialog->contents()->addWidget(WW<WLabel>(WString::tr("maximum_magnitude_label")));
    dialog->contents()->addWidget(slider(20));
    dialog->show();
  });
  d->setLabel({-1, 10});
}


void FilterByMagnitudeWidget::Private::setLabel(FilterByMagnitudeWidget::Range range)
{
  button->setText(WString::tr("magnitude_label").arg(range.minimum).arg(range.maximum));
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
//  d->magnitudeSlider->setMinimum(minimum*10);
}

void FilterByMagnitudeWidget::setMaximum(double maximum)
{
//  d->magnitudeSlider->setMaximum(maximum*10);
}

void FilterByMagnitudeWidget::setRange(const Range &range)
{
//   setMinimum(range.minimum);
//   setMaximum(range.maximum);
}

double FilterByMagnitudeWidget::magnitude() const
{
  return -1 ;
}

bool FilterByMagnitudeWidget::isMinimum() const
{
  return true;
}

bool FilterByMagnitudeWidget::isMaximum() const
{
  return false;
}


void FilterByMagnitudeWidget::resetDefaultValue()
{
//   d->magnitudeSlider->setValue(d->initialValue*10.);
//   d->checkValue();
  d->changed.emit();
}
