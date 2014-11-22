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

FilterByMagnitudeWidget::Private::Private(Range range, FilterByMagnitudeWidget *q): range(range), initialRange(range), q(q)
{
}


FilterByMagnitudeWidget::FilterByMagnitudeWidget(const FilterByMagnitudeWidget::Range& range, WContainerWidget* parent)
  : WCompositeWidget(parent), d(range, this)
{
  setImplementation(d->button = WW<WPushButton>().addCss("btn-sm btn-link filter-widget-link"));
  d->button->clicked().connect([=](WMouseEvent){
    WDialog *dialog = new WDialog;
    dialog->setClosable(true);
    dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).onClick([=](WMouseEvent){dialog->accept();}));

    auto slider = [=](double &value) {
      auto _slider = new WSlider(Horizontal);
      _slider->setWidth(400);
      _slider->setMinimum(-50);
      _slider->setMaximum(250);
      _slider->setValue(value*10);
      WText *label = WW<WText>().css("badge pull-right");
      auto set_label = [=](double value){
	label->setText(WString::fromUTF8(format("%.1f") % value));
      };
      set_label(value);
      _slider->valueChanged().connect([=,&value](int v, _n5){
	value = static_cast<double>(v) / 10.;
	set_label(value);
      });
      dialog->contents()->addWidget(label);
      dialog->contents()->addWidget(WW<WContainerWidget>().add(_slider));
      return _slider;
    };
    dialog->contents()->addWidget(WW<WLabel>(WString::tr("minimum_magnitude_label")));
    dialog->contents()->addWidget( slider(d->range.minimum));
    dialog->contents()->addWidget(WW<WLabel>(WString::tr("maximum_magnitude_label")));
    dialog->contents()->addWidget( slider(d->range.maximum));
    dialog->finished().connect([=](int result, _n5){
      if(result == WDialog::Rejected) {
	resetDefaultValue();
	return;
      }
      d->changed.emit();
      d->updateLabel();
    });
    dialog->show();
  });
  d->updateLabel();
}

FilterByMagnitudeWidget::~FilterByMagnitudeWidget()
{
}

void FilterByMagnitudeWidget::setRange(const FilterByMagnitudeWidget::Range& range)
{
  d->range = range;
  d->updateLabel();
}


void FilterByMagnitudeWidget::Private::updateLabel()
{
  button->setText(WString::tr("magnitude_label").arg((format("%.1f") % range.minimum).str()).arg((format("%.1f") % range.maximum).str()));
}

Signal<> &FilterByMagnitudeWidget::changed() const
{
  return d->changed;
}

FilterByMagnitudeWidget::Range FilterByMagnitudeWidget::value() const
{
  return d->range;
}


void FilterByMagnitudeWidget::resetDefaultValue()
{
  d->range = d->initialRange;
  d->changed.emit();
}
