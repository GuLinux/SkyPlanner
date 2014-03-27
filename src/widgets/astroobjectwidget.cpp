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

AstroObjectWidget::AstroObjectWidget(const AstroSessionObjectPtr &object, Session &session, bool addTitle, bool autoloadDSS, WContainerWidget *parent)
  : WCompositeWidget(parent), d(object, session, this)
{
  WContainerWidget *content = WW<WContainerWidget>().css("container-fluid");
  WContainerWidget *row = WW<WContainerWidget>().css("row");
  setImplementation(content);
  Dbo::Transaction t(session);
  if(addTitle) {
    auto names = NgcObject::namesByCatalogueImportance(t, object->ngcObject());
    content->addWidget(
          WW<WContainerWidget>().css("row").add(
            WW<WText>(format("<h2>%s</h2>") % boost::join(names, ", ")).css("text-center")
            )
          );
  }
  content->addWidget(row);
  auto dssPage = new DSSPage(object->ngcObject(), session, DSSPage::Options::embedded(autoloadDSS));
  //dssPage->setMaximumSize(400, 400);
  dssPage->addStyleClass("col-xs-5");
  WContainerWidget *info = WW<WContainerWidget>().css("col-xs-7");

  row->addWidget(dssPage);
  row->addWidget(info);
  info->addWidget(WW<WText>("hello world"));
}

AstroObjectWidget::~AstroObjectWidget()
{
}

