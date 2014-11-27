#ifndef FILTER_RANGE_H
#define FILTER_RANGE_H
#include "filterbyrangewidget_defs.h"
#include <Wt/WLabel>

template<>
struct FilterByRangeWidget<double>::Traits {
  std::string format(const double &t) const { return ::format("%.1f") % t; }
  int slider(const double &t) const { return t*10; }
  double value(int s) const { return static_cast<double>(s) / 10.; }
};

using namespace Wt;
using namespace WtCommons;
template<class T>
FilterByRangeWidget<T>::FilterByRangeWidget(const Range& value, const Range& outer, const Labels &labels, Wt::WContainerWidget* parent)
  : WCompositeWidget(parent), _value(value), _original(value), _outer(outer), _labels(labels)
{
    setImplementation(button = WW<WPushButton>().addCss("btn-sm btn-link filter-widget-link"));
    Traits traits;
    button->clicked().connect([=](WMouseEvent){
      WDialog *dialog = new WDialog;
      dialog->setClosable(true);
      dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).onClick([=](WMouseEvent){dialog->accept();}));
      dialog->setCaption(WString::tr(_labels.dialog_title));
      auto slider = [=](T &value) {
	auto _slider = new WSlider(Horizontal);
	_slider->setWidth(400);
	_slider->setMinimum(traits.slider(_outer.lower));
	_slider->setMaximum(traits.slider(_outer.upper));
	_slider->setValue(traits.slider(value));
	WText *label = WW<WText>().css("badge pull-right");
	auto set_label = [=](T value){
	  label->setText(WString::fromUTF8(traits.format(value)));
	};
	set_label(value);
	_slider->valueChanged().connect([=,&value](int v, _n5){
	  set_label(traits.value(v));
	  value = traits.value(v);
	});
	dialog->contents()->addWidget(label);
	dialog->contents()->addWidget(WW<WContainerWidget>().add(_slider));
	return _slider;
      };
      WSlider *lower, *upper;
      dialog->contents()->addWidget(WW<WLabel>(WString::tr(_labels.lower_slider)));
      dialog->contents()->addWidget(lower = slider(_value.lower));
      dialog->contents()->addWidget(WW<WLabel>(WString::tr(_labels.upper_slider)));
      dialog->contents()->addWidget(upper = slider(_value.upper));
      
      upper->valueChanged().connect([=](int v, _n5) {
	if( v < lower->value() ) {
	  lower->setValue(v);
	  lower->valueChanged().emit(v);
	}
      });
      lower->valueChanged().connect([=](int v, _n5) {
	if( v > upper->value() ) {
	  upper->setValue(v);
	  upper->valueChanged().emit(v);
	}
      });
      
      dialog->finished().connect([=](int result, _n5){
	if(result == WDialog::Rejected) {
	  resetDefaultValue();
	  return;
	}
	_changed.emit();
	updateLabel();
      });
      dialog->show();
    });
    
    updateLabel();
}

template<class T>
void FilterByRangeWidget<T>::updateLabel()
{
  Traits traits;
  button->setText(WString::tr(_labels.button).arg(traits.format(_value.lower)).arg(traits.format(_value.upper)));
}


#endif
