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

  explicit FilterByMagnitudeWidget(const Range &range, Wt::WContainerWidget *parent = 0);
  ~FilterByMagnitudeWidget();
  Wt::Signal<> &changed() const;
  Range value() const;
  void setRange(const Range &range);
  void resetDefaultValue();
private:
  D_PTR;
};

#endif // FILTERBYMAGNITUDEWIDGET_H


