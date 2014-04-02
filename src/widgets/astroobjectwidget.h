#ifndef ASTROOBJECTWIDGET_H
#define ASTROOBJECTWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <mutex>
class AstroSessionObject;
class Session;
class Ephemeris;
class Telescope;
class AstroSession;
class NgcObject;
class AstroObjectWidget : public Wt::WCompositeWidget {
public:
  explicit AstroObjectWidget(const Wt::Dbo::ptr<AstroSessionObject> &astroSession, Session &session, const Ephemeris &ephemeris, const Wt::Dbo::ptr<Telescope> &telescope, const std::shared_ptr<std::mutex> &downloadMutex = {}, const std::vector<Wt::WPushButton*> &actionButtons = {}, Wt::WContainerWidget *parent = 0);
  explicit AstroObjectWidget(const Wt::Dbo::ptr<NgcObject> &object, const Wt::Dbo::ptr<AstroSession> &astroSession, Session &session, const Ephemeris &ephemeris, const Wt::Dbo::ptr<Telescope> &telescope, const std::shared_ptr<std::mutex> &downloadMutex = {}, const std::vector<Wt::WPushButton*> &actionButtons = {}, Wt::WContainerWidget *parent = 0);
  ~AstroObjectWidget();
  void toggleInvert();
private:
  D_PTR;
};

#endif // ASTROOBJECTWIDGET_H


