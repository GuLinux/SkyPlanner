#ifndef FILTER_BY_RANGE_WIDGET_DEFS
#define FILTER_BY_RANGE_WIDGET_DEFS

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include "utils/format.h"
#include "Wt-Commons/wt_helpers.h"

#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WSlider>

template<class T>
class FilterByRangeWidget : public Wt::WCompositeWidget {
public:
  struct Range {
    T lower;
    T upper;
  };

  struct Traits {
    std::string format(const T &t) const { return ::format("%d") % t; }
    int slider(const T &t) const { return t; }
    T value(int s) const { return s; }
  };
  
  struct Labels {
    std::string button;
    std::string dialog_title;
    std::string lower_slider;
    std::string upper_slider;
    Labels(const std::string &button, const std::string &dialog_title, const std::string &lower_slider, const std::string &upper_slider)
      : button(button), dialog_title(dialog_title), lower_slider(lower_slider), upper_slider(upper_slider) {}
  };
  
  explicit FilterByRangeWidget(const Range &value, const Range &outer, const Labels &labels, Wt::WContainerWidget *parent = 0);
  Wt::Signal<> &changed() { return _changed; }
  void resetDefaultValue() { _value = _original; }
  Range value() const { return _value; }
  void setValue(const Range &range) { _value = range; updateLabel(); }
private:
  Range _value;
  const Range _original;
  const Range _outer;
  Labels _labels;
  Wt::Signal<> _changed;

  Wt::WPushButton *button;
  void updateLabel();
};

#endif
