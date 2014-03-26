#ifndef ASTROOBJECTWIDGET_H
#define ASTROOBJECTWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>

class AstroSessionObject;
class Session;
class AstroObjectWidget : public Wt::WCompositeWidget {
public:
  explicit AstroObjectWidget(const Wt::Dbo::ptr<AstroSessionObject> &object, Session &session, bool addTitle = true, Wt::WContainerWidget *parent = 0);
  ~AstroObjectWidget();
private:
  D_PTR;
};

#endif // ASTROOBJECTWIDGET_H


