/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
#ifndef FILTER_BY_RANGE_WIDGET_DEFS
#define FILTER_BY_RANGE_WIDGET_DEFS

#include <Wt/WCompositeWidget>
#include <Wt/WContainerWidget>
#include "utils/format.h"
#include "Wt-Commons/wt_helpers.h"
#include <types.h>

#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WSlider>
#include <boost/date_time.hpp>

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

template<>
struct Traits<Angle> {
  typedef std::function<std::string(Angle)> Format;
  Traits(Format format = [](const Angle &angle) { return angle.printable(); }) : _format(format) {}
  std::string format(const Angle &t) const { return _format(t); }
  int slider(const Angle &t) const { return t.degrees() * 50000; }
  Angle value(int s) const { return Angle::degrees(static_cast<double>(s) / 50000); }
  Format _format;
};
template<>
struct Traits<boost::posix_time::ptime> {
  typedef std::function<std::string(boost::posix_time::ptime)> Format;
  Traits(const boost::posix_time::ptime &start, Format formatter = [](const boost::posix_time::ptime &t) { return boost::posix_time::to_simple_string(t); }) : _start(start), _formatter(formatter) {}
  std::string format(const boost::posix_time::ptime &t) const { return _formatter(t); }
  int slider(const boost::posix_time::ptime &t) const { return (t - _start).total_seconds(); }
  boost::posix_time::ptime value(int s) const { return _start + boost::posix_time::seconds(s); }
  boost::posix_time::ptime _start;
  Format _formatter;
};
}

template<class T>
class FilterByRangeWidget : public Wt::WCompositeWidget {
public:
  typedef FilterByRange::Traits<T> Traits;
  typedef FilterByRange::Range<T> Range;
  struct Labels {
    std::string button;
    std::string dialog_title;
    std::string lower_slider;
    std::string upper_slider;
    Labels(const std::string &button, const std::string &dialog_title, const std::string &lower_slider, const std::string &upper_slider)
      : button(button), dialog_title(dialog_title), lower_slider(lower_slider), upper_slider(upper_slider) {}
  };
  
  explicit FilterByRangeWidget(const Range &outer, const Labels &labels, const std::shared_ptr<Traits> &traits = std::make_shared<Traits>(), Wt::WContainerWidget *parent = 0);
  Wt::Signal<> &changed() { return _changed; }
  void resetDefaultValue() { _value = _original; _changed.emit(); updateLabel();}
  Range value() const { return _value; }
  void setValue(const Range &range) { _value = range; _original = range; updateLabel(); }
  void setOuterRange(const Range &range) { _outer = range; }
private:
  Range _value;
  Range _original;
  Range _outer;
  Labels _labels;
  Wt::Signal<> _changed;

  Wt::WPushButton *button;
  void updateLabel();
  std::shared_ptr<Traits> traits;
};

#endif
