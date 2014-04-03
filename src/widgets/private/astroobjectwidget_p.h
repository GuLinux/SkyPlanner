#ifndef ASTROOBJECTWIDGET_P_H
#define ASTROOBJECTWIDGET_P_H

#include "widgets/astroobjectwidget.h"
#include "models/Models"
#include "session.h"
class DSSPage;
class AstroObjectWidget::Private
{
public:
  Private(const AstroSessionObjectPtr &astroSessionObject, const AstroSessionPtr &astroSession, const NgcObjectPtr &ngcObject, Session &session, const Timezone &timezone, const TelescopePtr &telescope, const std::shared_ptr<std::mutex> &downloadMutex, const std::vector<Wt::WPushButton*> &actionButtons, AstroObjectWidget *q);

  void init();

  AstroSessionObjectPtr astroSessionObject;
  AstroSessionPtr astroSession;
  NgcObjectPtr ngcObject;
  Session &session;
  Timezone timezone;
  TelescopePtr telescope;
  std::shared_ptr<std::mutex> downloadMutex;
  std::vector<Wt::WPushButton*> actionButtons;
  

  DSSPage *dssPage;
  Wt::WContainerWidget *content;
  Wt::WContainerWidget *expanded;
  Wt::WWidget *collapsed;
  Wt::WTemplate *info;
private:
  AstroObjectWidget *q;
};

#endif // ASTROOBJECTWIDGET_H


