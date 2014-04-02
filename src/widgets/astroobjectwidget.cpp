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
AstroObjectWidget::Private::Private(AstroObjectWidget *q): q(q)
{
}

AstroObjectWidget::AstroObjectWidget(const AstroSessionObjectPtr &object, Session &session, const Ephemeris &ephemeris, const TelescopePtr &telescope, const shared_ptr<mutex> &downloadMutex, const vector<Wt::WPushButton*> &actionButtons, WContainerWidget *parent)
  : WCompositeWidget(parent), d(this)
{
  d->init(object, object->astroSession(), object->ngcObject(), session, ephemeris, telescope, downloadMutex, actionButtons);
}

AstroObjectWidget::AstroObjectWidget(const NgcObjectPtr &ngcObject, const AstroSessionPtr &astroSession, Session &session, const Ephemeris &ephemeris, const TelescopePtr &telescope, const shared_ptr<mutex> &downloadMutex, const vector<Wt::WPushButton*> &actionButtons, WContainerWidget *parent)
  : WCompositeWidget(parent), d(this)
{
  d->init({}, astroSession, ngcObject, session, ephemeris, telescope, downloadMutex, actionButtons);
}



void AstroObjectWidget::Private::init(const AstroSessionObjectPtr &astroSessionObject, const AstroSessionPtr &astroSession, const NgcObjectPtr &ngcObject, Session &session, const Ephemeris &ephemeris, const TelescopePtr &telescope, const std::shared_ptr<std::mutex> &downloadMutex, const std::vector<Wt::WPushButton*> &actionButtons)
{
  WContainerWidget *content = WW<WContainerWidget>().css("container-fluid astroobjectwidget");
  WContainerWidget *row = WW<WContainerWidget>().css("row print-no-break");
  q->setImplementation(content);
  Dbo::Transaction t(session);
  auto names = NgcObject::namesByCatalogueImportance(t, ngcObject);
  content->addWidget(
    WW<WContainerWidget>().css("row print-no-break hidden-print").add(
    WW<WText>(format("<h4>%s</h4>") % boost::join(names, ", ")).css("astroobject_title text-center")
   )
  );
  content->addWidget(row);
  dssPage = new DSSPage(ngcObject, session, DSSPage::Options::embedded(downloadMutex));
  //dssPage->setMaximumSize(400, 400);
  dssPage->addStyleClass("col-xs-6 col-lg-5");
  WTemplate *info = WW<WTemplate>(WString::tr("astroobjectwidget")).css("col-xs-6 col-lg-7");
  info->addFunction( "tr", &WTemplate::Functions::tr);
  info->bindString("title", WString::fromUTF8(boost::join(names, ", ")));
  info->bindString("ar", ngcObject->coordinates().rightAscension.printable(Angle::Hourly));
  info->bindString("dec", Utils::htmlEncode( WString::fromUTF8(ngcObject->coordinates().declination.printable()) ));
  info->bindString("type", ngcObject->typeDescription());
  info->bindString("constellation", WString::fromUTF8(ngcObject->constellation().name));
  info->bindString("constellation_abbrev", WString::fromUTF8(ngcObject->constellation().abbrev));
  info->bindString("angular_size", Utils::htmlEncode( WString::fromUTF8( Angle::degrees(ngcObject->angularSize()).printable() )) );
  info->bindString("magnitude", Utils::htmlEncode( ngcObject->magnitude() > 90. ? "N/A" : (format("%.1f") % ngcObject->magnitude()).str() ));

  auto bestAltitude =  AstroSessionObject::bestAltitude(astroSession, ngcObject, ephemeris, 1);

  info->bindString("best_altitude_when", WDateTime::fromPosixTime( ephemeris.timezone().fix(bestAltitude.when)).time().toString());
  info->bindString("best_altitude", Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) );
  info->bindWidget("difficulty", new ObjectDifficultyWidget{ngcObject, telescope, bestAltitude.coordinates.altitude.degrees() } );
  info->setCondition("has-catalogues-descriptions", ngcObject->descriptions().size() > 0);
  if(info->conditionValue("has-catalogues-descriptions")) {
    info->bindWidget("catalogues-description", new CataloguesDescriptionWidget{ngcObject->descriptions()});
  }


  info->setCondition("has-custom-description", astroSessionObject && astroSessionObject->description().size() > 0);
  if(info->conditionValue("has-custom-description")) {
    info->bindString("custom-description", WString::fromUTF8(astroSessionObject->description()));
  }


  info->setCondition("have-actions", actionButtons.size() > 0);
  if(info->conditionValue("have-actions")) {
    WToolBar *toolbar = new WToolBar;
    for(auto btn: actionButtons)
      toolbar->addButton(btn);
    
    info->bindWidget("actions", toolbar);
  }
 row->addWidget(dssPage);
  row->addWidget(info);
}

void AstroObjectWidget::toggleInvert()
{
  d->dssPage->toggleInvert();
}

AstroObjectWidget::~AstroObjectWidget()
{
}

