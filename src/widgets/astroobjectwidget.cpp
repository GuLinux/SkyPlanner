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
#include "widgets/objectnameswidget.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
AstroObjectWidget::Private::Private(const AstroSessionObjectPtr &astroSessionObject, const AstroSessionPtr &astroSession, const NgcObjectPtr &ngcObject, Session &session, const Timezone &timezone, const TelescopePtr &telescope, const shared_ptr<mutex> &downloadMutex, const std::vector<WPushButton*> &actionButtons, AstroObjectWidget *q)
  : astroSessionObject(astroSessionObject), astroSession(astroSession), ngcObject(ngcObject), session(session), timezone(timezone), telescope(telescope), downloadMutex(downloadMutex), actionButtons(actionButtons), q(q)
{
}

AstroObjectWidget::AstroObjectWidget(const AstroSessionObjectPtr &object, Session &session, const Timezone &timezone, const TelescopePtr &telescope, const shared_ptr<mutex> &downloadMutex, const vector<Wt::WPushButton*> &actionButtons, WContainerWidget *parent)
  : WCompositeWidget(parent), d(object, object->astroSession(), object->ngcObject(), session, timezone, telescope, downloadMutex, actionButtons, this)
{
  setImplementation(d->content = WW<WContainerWidget>());
  d->init();
}

AstroObjectWidget::AstroObjectWidget(const NgcObjectPtr &ngcObject, const AstroSessionPtr &astroSession, Session &session, const Timezone &timezone, const TelescopePtr &telescope, const shared_ptr<mutex> &downloadMutex, const vector<Wt::WPushButton*> &actionButtons, WContainerWidget *parent)
  : WCompositeWidget(parent), d(AstroSessionObjectPtr{}, astroSession, ngcObject, session, timezone, telescope, downloadMutex, actionButtons, this)
{
  setImplementation(d->content = WW<WContainerWidget>());
  d->init();
}


void AstroObjectWidget::reload()
{
  for(auto actionButton: d->actionButtons)
    d->actionsToolbar->removeWidget(actionButton);
  d->init();
}


void AstroObjectWidget::Private::init()
{
  content->clear();
  content->addStyleClass("container-fluid astroobjectwidget");
  WContainerWidget *row = WW<WContainerWidget>().css("row print-no-break");
  expanded = WW<WContainerWidget>();
  auto names = [=] { return new ObjectNamesWidget(ngcObject, session, astroSession, ObjectNamesWidget::Printable); }; 
  collapsed = WW<WTemplate>("<small class=\"text-center astroobject_title\"><b>${title-widget} ${expand}</b></small>").setHidden(true).bindWidget("title-widget", names() ).bindWidget("expand", WW<WPushButton>(WString::tr("buttons_expand")).css("btn-xs hidden-print pull-right").onClick([=](WMouseEvent){ q->setCollapsed(false); }));
  Dbo::Transaction t(session);
  expanded->addWidget(WW<WTemplate>("<h4 class=\"row print-no-break hidden-print astroobject_title text-center\">${title-widget}</h4>").bindWidget("title-widget", names()) );
  expanded->addWidget(row);
  content->addWidget(collapsed);
  content->addWidget(expanded);
  dssPage = new DSSPage(ngcObject, session, DSSPage::Options::embedded(downloadMutex));
  info = WW<WTemplate>(WString::tr("astroobjectwidget")).css("col-xs-6 col-lg-7");
  info->addFunction( "tr", &WTemplate::Functions::tr);
  info->bindWidget("title", names());
  info->bindString("ar", ngcObject->coordinates().rightAscension.printable(Angle::Hourly));
  info->bindString("dec", Utils::htmlEncode( WString::fromUTF8(ngcObject->coordinates().declination.printable()) ));
  info->bindString("type", ngcObject->typeDescription());
  info->bindString("constellation", WString::fromUTF8(ngcObject->constellation().name));
  info->bindString("constellation_abbrev", WString::fromUTF8(ngcObject->constellation().abbrev));
  info->bindString("angular_size", Utils::htmlEncode( WString::fromUTF8( Angle::degrees(ngcObject->angularSize()).printable() )) );
  info->bindString("magnitude", Utils::htmlEncode( ngcObject->magnitude() > 90. ? "N/A" : (format("%.1f") % ngcObject->magnitude()).str() ));

  info->setCondition("have-ephemeris", astroSession);
  if(astroSession) {
    Ephemeris ephemeris(astroSession->position(), timezone);
    auto bestAltitude =  AstroSessionObject::bestAltitude(astroSession, ngcObject, ephemeris);

    info->bindString("best_altitude_when", bestAltitude.when.str() );
    info->bindString("best_altitude", Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) );
    info->bindWidget("difficulty", new ObjectDifficultyWidget{ngcObject, telescope, bestAltitude.coordinates.altitude.degrees() } );
  }
  info->setCondition("has-catalogues-descriptions", ngcObject->descriptions().size() > 0);
  if(info->conditionValue("has-catalogues-descriptions")) {
    info->bindWidget("catalogues-description", new CataloguesDescriptionWidget{ngcObject->descriptions()});
  }
  if(!astroSessionObject)
    astroSessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(ngcObject.id()).where("astro_session_id = ?").bind(astroSession.id());

  info->setCondition("has-custom-description", astroSessionObject && astroSessionObject->description().size() > 0);
  if(info->conditionValue("has-custom-description")) {
    info->bindString("custom-description", WString::fromUTF8(astroSessionObject->description()));
  }


  info->setCondition("have-actions", actionButtons.size() > 0);
  if(info->conditionValue("have-actions")) {
    actionsToolbar = WW<WContainerWidget>().css("btn-group");
    for(auto btn: actionButtons)
      actionsToolbar->addWidget(btn);
    
    info->bindWidget("actions", actionsToolbar);
  }
  row->addWidget(WW<WContainerWidget>().css("col-xs-6 col-lg-5").add(dssPage));
  row->addWidget(info);
}

void AstroObjectWidget::toggleInvert()
{
  d->dssPage->toggleInvert();
}

void AstroObjectWidget::setDSSVisible(bool visible)
{
  d->dssPage->setHidden(!visible);
}

bool AstroObjectWidget::isDSSVisible() const
{
  return d->dssPage->isVisible();
}

void AstroObjectWidget::setCollapsed(bool collapsed)
{
  d->collapsed->setHidden(!collapsed);
  d->expanded->setHidden(collapsed);
}

bool AstroObjectWidget::isCollapsed() const
{
  return d->collapsed->isVisible();
}


AstroObjectWidget::~AstroObjectWidget()
{
}

