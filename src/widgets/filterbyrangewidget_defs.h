#ifndef FILTER_BY_RANGE_WIDGET_DEFS
#define FILTER_BY_RANGE_WIDGET_DEFS

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include "utils/format.h"
#include "Wt-Commons/wt_helpers.h"

#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WSlider>

namespace FilterByRange {
  template<class T>
  struct Range {
    T lower;
    T upper;
  };

  template<class T>
  struct Traits {
    std::string format(const T &t) const { return ::format("%d") % t; }
    int slider(const T &t) const { return t; }
    T value(int s) const { return s; }
  };
  
  
template<>
struct Traits<double> {
  Traits(unsigned int decimals = 1) : decimals(decimals) {}
  std::string format(const double &t) const {
    std::string format_string = ::format("%%.%df") % decimals;
    return ::format(format_string) % t;
  }
  int slider(const double &t) const { return t*pow(10, decimals); }
  double value(int s) const { return static_cast<double>(s) / pow(10, decimals); }
private:
  unsigned int decimals;
};
}

template<class T>
class FilterByRangeWidget : public Wt::WCompositeWidget {
public:
  struct Labels {
    std::string button;
    std::string dialog_title;
    std::string lower_slider;
    std::string upper_slider;
    Labels(const std::string &button, const std::string &dialog_title, const std::string &lower_slider, const std::string &upper_slider)
      : button(button), dialog_title(dialog_title), lower_slider(lower_slider), upper_slider(upper_slider) {}
  };
  
  explicit FilterByRangeWidget(const FilterByRange::Range<T> &value, const FilterByRange::Range<T> &outer, const Labels &labels, const FilterByRange::Traits<T> &traits = {}, Wt::WContainerWidget *parent = 0);
  Wt::Signal<> &changed() { return _changed; }
  void resetDefaultValue() { _value = _original; }
  FilterByRange::Range<T> value() const { return _value; }
  void setValue(const FilterByRange::Range<T> &range) { _value = range; updateLabel(); }
private:
  FilterByRange::Range<T> _value;
  const FilterByRange::Range<T> _original;
  const FilterByRange::Range<T> _outer;
  Labels _labels;
  Wt::Signal<> _changed;

  Wt::WPushButton *button;
  void updateLabel();
  FilterByRange::Traits<T> traits;
};

#endif
