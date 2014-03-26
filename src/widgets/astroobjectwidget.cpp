#include "astroobjectwidget.h"
#include "private/astroobjectwidget_p.h"
#include "Wt-Commons/wt_helpers.h"

#include <utils/d_ptr_implementation.h>
#include "utils/format.h"
#include <boost/algorithm/string.hpp>
#include <Wt/WText>
#include "widgets/dsspage.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
AstroObjectWidget::Private::Private(const AstroSessionObjectPtr &object, Session &session, AstroObjectWidget *q): object(object), session(session), q(q)
{
}

AstroObjectWidget::AstroObjectWidget(const AstroSessionObjectPtr &object, Session &session, bool addTitle, WContainerWidget *parent)
  : WCompositeWidget(parent), d(object, session, this)
{
  WContainerWidget *content = new WContainerWidget;
  setImplementation(content);
  Dbo::Transaction t(session);
  if(addTitle) {
    auto names = NgcObject::namesByCatalogueImportance(t, object->ngcObject());
    content->addWidget(WW<WText>(format("<h2>%s</h2>") % boost::join(names, ", ")));
  }
  DSSPage::Options dssOptions;
  dssOptions.showClose = false;
  auto dssPage = new DSSPage(object->ngcObject(), session, dssOptions);
  dssPage->setMaximumSize(400, 400);
  content->addWidget(dssPage);
}

AstroObjectWidget::~AstroObjectWidget()
{
}

