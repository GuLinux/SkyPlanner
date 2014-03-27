#include "astroobjectwidget.h"
#include "private/astroobjectwidget_p.h"
#include "Wt-Commons/wt_helpers.h"

#include <utils/d_ptr_implementation.h>
#include "utils/format.h"
#include <boost/algorithm/string.hpp>
#include <Wt/WText>
#include "widgets/dsspage.h"
#include <Wt/WTemplate>
#include <Wt/Utils>
#include "ephemeris.h"
#include "widgets/cataloguesdescriptionwidget.h"
#include "widgets/objectdifficultywidget.h"
#include <Wt/WToolBar>
#include <Wt/WPushButton>

using namespace std;
using namespace Wt;
using namespace WtCommons;
AstroObjectWidget::Private::Private(const AstroSessionObjectPtr &object, Session &session, AstroObjectWidget *q): object(object), session(session), q(q)
{
}

AstroObjectWidget::AstroObjectWidget(const AstroSessionObjectPtr &object, Session &session, const Ephemeris &ephemeris, const TelescopePtr &telescope, bool addTitle, const shared_ptr<mutex> &downloadMutex, const vector<AstroObjectWidget::ActionButton> &actionButtons, WContainerWidget *parent)
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
  auto dssPage = new DSSPage(object->ngcObject(), session, DSSPage::Options::embedded(downloadMutex));
  //dssPage->setMaximumSize(400, 400);
  dssPage->addStyleClass("col-xs-4");
  WTemplate *info = WW<WTemplate>(WString::tr("astroobjectwidget")).css("col-xs-8");
  info->addFunction( "tr", &WTemplate::Functions::tr);
  info->bindString("ar", object->coordinates().rightAscension.printable(Angle::Hourly));
  info->bindString("dec", Utils::htmlEncode( WString::fromUTF8(object->coordinates().declination.printable()) ));
  info->bindString("type", object->ngcObject()->typeDescription());
  info->bindString("constellation", WString::fromUTF8(object->ngcObject()->constellation().name));
  info->bindString("constellation_abbrev", WString::fromUTF8(object->ngcObject()->constellation().abbrev));
  info->bindString("angular_size", Utils::htmlEncode( WString::fromUTF8( Angle::degrees(object->ngcObject()->angularSize()).printable() )) );
  info->bindString("magnitude", Utils::htmlEncode( object->ngcObject()->magnitude() > 90. ? "N/A" : (format("%.1f") % object->ngcObject()->magnitude()).str() ));
  auto bestAltitude = object->bestAltitude(ephemeris, 1);
  info->bindString("best_altitude_when", WDateTime::fromPosixTime( ephemeris.timezone().fix(bestAltitude.when)).time().toString());
  info->bindString("best_altitude", Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) );
  info->bindWidget("difficulty", new ObjectDifficultyWidget{object->ngcObject(), telescope, bestAltitude.coordinates.altitude.degrees() } );
  info->setCondition("has-catalogues-descriptions", object->ngcObject()->descriptions().size() > 0);
  if(info->conditionValue("has-catalogues-descriptions")) {
    info->bindWidget("catalogues-description", new CataloguesDescriptionWidget{object->ngcObject()->descriptions()});
  }
  info->setCondition("has-custom-description", object->ngcObject()->descriptions().size() > 0);
  if(info->conditionValue("has-custom-description")) {
    info->bindString("custom-description", object->description());
  }

  info->setCondition("have-actions", actionButtons.size() > 0);
  if(info->conditionValue("have-actions")) {
    WToolBar *toolbar = new WToolBar;
    for(auto btn: actionButtons)
      toolbar->addButton(WW<WPushButton>(btn.text).css("btn-xs").addCss(btn.cssClass).onClick([=](WMouseEvent){ btn.onClick(); }));
    
    info->bindWidget("actions", toolbar);
  }
 row->addWidget(dssPage);
  row->addWidget(info);
}

AstroObjectWidget::~AstroObjectWidget()
{
}

