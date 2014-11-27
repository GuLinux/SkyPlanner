#ifndef FILTER_RANGE_H
#define FILTER_RANGE_H
#include "filterbyrangewidget_defs.h"
#include <Wt/WLabel>

using namespace Wt;
using namespace std;
using namespace WtCommons;
using namespace FilterByRange;
template<class T>
FilterByRangeWidget<T>::FilterByRangeWidget(const Range<T>& outer, const Labels &labels, const shared_ptr<Traits> &traits, Wt::WContainerWidget* parent)
  : WCompositeWidget(parent), _value(outer), _original(outer), _outer(outer), _labels(labels), traits(traits)
{
    setImplementation(button = WW<WPushButton>().addCss("btn-sm btn-link filter-widget-link"));
    button->clicked().connect([=](WMouseEvent){
      WDialog *dialog = new WDialog;
      dialog->setClosable(true);
      dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).onClick([=](WMouseEvent){dialog->accept();}));
      dialog->setCaption(WString::tr(_labels.dialog_title));
      auto slider = [=](T &value) {
	auto _slider = new WSlider(Horizontal);
	_slider->setWidth(500);
	_slider->setMinimum(traits->slider(_outer.lower));
	_slider->setMaximum(traits->slider(_outer.upper));
	_slider->setValue(traits->slider(value));
	WText *label = WW<WText>().css("badge pull-right");
	auto set_label = [=](T value){
	  label->setText(WString::fromUTF8(traits->format(value)));
	};
	set_label(value);
	_slider->valueChanged().connect([=,&value](int v, _n5){
	  set_label(traits->value(v));
	  value = traits->value(v);
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
  button->setText(WString::tr(_labels.button).arg(WString::fromUTF8(traits->format(_value.lower))).arg(WString::fromUTF8(traits->format(_value.upper))));
}


#endif
