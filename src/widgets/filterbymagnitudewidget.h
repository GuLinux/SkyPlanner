#ifndef FILTERBYMAGNITUDEWIDGET_H
#define FILTERBYMAGNITUDEWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>

class FilterByMagnitudeWidget : public Wt::WCompositeWidget {
public:
  struct Range {
    double minimum;
    double maximum;
  };
  struct Options {
    Wt::WString minimumValueText;
    Wt::WString maximumValueText;
    Wt::WString labelText;
  };

  explicit FilterByMagnitudeWidget(const Options &options, const Range &range, double initialValue, Wt::WContainerWidget *parent = 0);
  explicit FilterByMagnitudeWidget(const Options &options, const Range &range, Wt::WContainerWidget *parent = 0);
  ~FilterByMagnitudeWidget();
  void setMinimum(double minimum);
  void setMaximum(double maximum);
  void setRange(const Range &range);
  double magnitude() const;
  bool isMinimum() const;
  bool isMaximum() const;
  Wt::Signal<> &changed() const;
  void resetDefaultValue();
private:
  D_PTR;
};

#endif // FILTERBYMAGNITUDEWIDGET_H


