#ifndef FILTERBYOBSERVEDWIDGET_H
#define FILTERBYOBSERVEDWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <boost/logic/tribool.hpp>
class FilterByObservedWidget : public Wt::WCompositeWidget {
public:
  explicit FilterByObservedWidget(const Wt::WString &label, Wt::WContainerWidget *parent = 0);
  ~FilterByObservedWidget();
  boost::logic::tribool value() const;
  void resetDefaultValue();
  Wt::Signal<> &changed() const;

private:
  D_PTR;
};

#endif // FILTERBYOBSERVEDWIDGET_H


