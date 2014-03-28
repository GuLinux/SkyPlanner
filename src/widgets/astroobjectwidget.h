#ifndef ASTROOBJECTWIDGET_H
#define ASTROOBJECTWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <mutex>
class AstroSessionObject;
class Session;
class Ephemeris;
class Telescope;
class AstroObjectWidget : public Wt::WCompositeWidget {
public:
  struct ActionButton {
    Wt::WString text;
    std::string cssClass;
    std::function<void()> onClick;
  };
  explicit AstroObjectWidget(const Wt::Dbo::ptr<AstroSessionObject> &object, Session &session, const Ephemeris &ephemeris, const Wt::Dbo::ptr<Telescope> &telescope, bool addTitle = true, const std::shared_ptr<std::mutex> &downloadMutex = {}, const std::vector<ActionButton> &actionButtons = {}, Wt::WContainerWidget *parent = 0);
  ~AstroObjectWidget();
  void toggleInvert();
private:
  D_PTR;
};

#endif // ASTROOBJECTWIDGET_H


