#ifndef ASTROOBJECTWIDGET_P_H
#define ASTROOBJECTWIDGET_P_H

#include "widgets/astroobjectwidget.h"
#include "models/Models"
#include "session.h"
class AstroObjectWidget::Private
{
public:
  Private(const AstroSessionObjectPtr &object, Session &session, AstroObjectWidget *q);
  AstroSessionObjectPtr object;
  Session &session;
private:
  AstroObjectWidget *q;
};

#endif // ASTROOBJECTWIDGET_H


