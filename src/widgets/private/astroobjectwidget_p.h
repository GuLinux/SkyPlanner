#ifndef ASTROOBJECTWIDGET_P_H
#define ASTROOBJECTWIDGET_P_H

#include "widgets/astroobjectwidget.h"
#include "models/Models"
#include "session.h"
class DSSPage;
class AstroObjectWidget::Private
{
public:
  Private(AstroObjectWidget *q);

  void init(const AstroSessionObjectPtr &astroSessionObject, const AstroSessionPtr &astroSession, const NgcObjectPtr &ngcObject, Session &session, const Ephemeris &ephemeris, const TelescopePtr &telescope, bool addTitle, const std::shared_ptr<std::mutex> &downloadMutex, const std::vector<Wt::WPushButton*> &actionButtons);
  DSSPage *dssPage;
private:
  AstroObjectWidget *q;
};

#endif // ASTROOBJECTWIDGET_H


