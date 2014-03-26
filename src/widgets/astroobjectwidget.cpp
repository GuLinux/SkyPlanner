#include "astroobjectwidget.h"
#include "private/astroobjectwidget_p.h"
#include "Wt-Commons/wt_helpers.h"

#include <utils/d_ptr_implementation.h>

using namespace std;
using namespace Wt;
using namespace WtCommons;
AstroObjectWidget::Private::Private(const AstroSessionObjectPtr &object, Session &session, AstroObjectWidget *q): object(object), session(session), q(q)
{
}

AstroObjectWidget::AstroObjectWidget(const AstroSessionObjectPtr &object, Session &session, WContainerWidget *parent)
  : WCompositeWidget(parent), d(object, session, this)
{
}

AstroObjectWidget::~AstroObjectWidget()
{
}

