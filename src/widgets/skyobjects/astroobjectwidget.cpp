/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
#include "astroobjectwidget.h"
#include "private/astroobjectwidget_p.h"
#include "wt_helpers.h"
#include <whtmltxt.h>

#include "utils/format.h"
#include <boost/algorithm/string.hpp>
#include <Wt/WText>
#include "widgets/dss/dsspage.h"
#include <Wt/WTemplate>
#include <Wt/Utils>
#include "ephemeris.h"
#include "cataloguesdescriptionwidget.h"
#include "objectdifficultywidget.h"
#include <Wt/WToolBar>
#include <Wt/WPushButton>
#include "objectnameswidget.h"
#include "skyplanner.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
AstroObjectWidget::Private::Private(const AstroGroup &astroGroup, Session &session, const shared_ptr<mutex> &downloadMutex, const std::vector<WPushButton*> &actionButtons, AstroObjectWidget *q)
  : astroGroup(astroGroup), session(session), downloadMutex(downloadMutex), actionButtons(actionButtons), q(q)
{
}

AstroObjectWidget::AstroObjectWidget(const AstroGroup &astroGroup, Session &session, const shared_ptr<mutex> &downloadMutex, const vector<Wt::WPushButton*> &actionButtons, WContainerWidget *parent)
  : WCompositeWidget(parent), dptr(astroGroup, session, downloadMutex, actionButtons, this)
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
  auto ngcObject = astroGroup.object();
  auto astroSession = astroGroup.astroSession();
  content->clear();
  content->addStyleClass("container-fluid astroobjectwidget");
  WContainerWidget *row = WW<WContainerWidget>().css("row print-no-break");
  expanded = WW<WContainerWidget>();
  auto names = [=] { return new ObjectNamesWidget(ngcObject, session, nullptr, ObjectNamesWidget::Printable); }; 
  collapsed = WW<WTemplate>("<small class=\"text-center astroobject_title\"><b>${title-widget} ${expand}</b></small>").setHidden(true).bindWidget("title-widget", names() ).bindWidget("expand", WW<WPushButton>(WString::tr("buttons_expand")).css("btn-xs hidden-print pull-right").onClick([=](WMouseEvent){ q->setCollapsed(false); }));
  Dbo::Transaction t(session);
  expanded->addWidget(WW<WTemplate>("<h4 class=\"row print-no-break hidden-print astroobject_title text-center\">${title-widget}</h4>").bindWidget("title-widget", names()) );
  expanded->addWidget(row);
  content->addWidget(collapsed);
  content->addWidget(expanded);
  dssPage = new DSSPage(ngcObject, session, DSSPage::Options::embedded(downloadMutex));
  info = WW<WTemplate>(WString::tr("astroobjectwidget")).css("col-xs-7 col-lg-7");
  info->addFunction( "tr", &WTemplate::Functions::tr);
  info->bindWidget("title", names());
  info->bindWidget("ar", new WHTMLTxt{ngcObject->coordinates().rightAscension.printable(Angle::Hourly)});
  info->bindWidget("dec", new WHTMLTxt{ngcObject->coordinates().declination.printable()});
  info->bindWidget("type", new WHTMLTxt{ngcObject->typeDescription()});
  info->bindWidget("constellation", new WHTMLTxt{ngcObject->constellation().name});
  info->bindWidget("constellation_abbrev", new WHTMLTxt{ngcObject->constellation().abbrev});
  info->setCondition("have-angular-size", ngcObject->angularSize() > 0);
  info->bindWidget("angular_size", new WHTMLTxt{Angle::degrees(ngcObject->angularSize()).printable() });
  info->setCondition("have-magnitude", ngcObject->magnitude() <= 90.);
  info->bindWidget("magnitude", new WHTMLTxt{(format("%.1f") % ngcObject->magnitude()).str() });
  
  bool have_eyepieces = ngcObject->angularSize() > 0 && astroSession && astroSession->user() == session.user() && session.user()->instruments_ok() && astroGroup.telescope;
  if(have_eyepieces) {
    std::vector<OpticalSetup> fieldInfos;
    std::transform(begin(session.user()->eyepieces()), end(session.user()->eyepieces()), back_inserter(fieldInfos), [=](const EyepiecePtr &e){ return OpticalSetup{astroGroup.telescope, e}; } );
    auto angularSize = Angle::degrees(ngcObject->angularSize());
    fieldInfos.erase(remove_if(fieldInfos.begin(), fieldInfos.end(), [=](const OpticalSetup &o) { return o.fov() < angularSize * 2. || o.fov() > angularSize * 5.; } ), fieldInfos.end() );
    
    auto fovIndex = [=] (const OpticalSetup &o) { return abs( (angularSize * 3. - o.fov()).degrees() ); };
    std::sort(fieldInfos.begin(), fieldInfos.end(), [=](const OpticalSetup &a, const OpticalSetup &b){
      return fovIndex(a) < fovIndex(b);
    });

    info->setCondition("have-eyepiece", !fieldInfos.empty() );
    if(info->conditionValue("have-eyepiece")) {
      info->bindWidget("eyepiece", new WHTMLTxt{fieldInfos[0].eyepiece()->name()});
    }
  }

  info->setCondition("have-ephemeris", static_cast<bool>(astroSession));
  if(astroSession) {
    Ephemeris ephemeris(astroSession->position(), astroGroup.timezone);
    auto bestAltitude =  AstroSessionObject::bestAltitude(astroSession, ngcObject, ephemeris);

    info->bindWidget("best_altitude_when", new WHTMLTxt{bestAltitude.when.str()});
    info->bindWidget("best_altitude", new WHTMLTxt{bestAltitude.coordinates.altitude.printable() } );
    auto riseSet = [](const Ephemeris::RiseTransitSet &r, const DateTime &t) {
      if(r.type == Ephemeris::RiseTransitSet::Normal)
	return WString::fromUTF8(t.str(DateTime::HoursAndMinutes));
      return r.type == Ephemeris::RiseTransitSet::CircumPolar ? WString::tr("circumpolar") : WString::tr("never_rises");
    };
    info->bindWidget("rst", new WHTMLTxt{WString("{1} - {2} - {3}") 
      .arg(riseSet(bestAltitude.rst, bestAltitude.rst.rise))
      .arg(bestAltitude.rst.transit.str(DateTime::HoursAndMinutes))
      .arg(riseSet(bestAltitude.rst, bestAltitude.rst.set)) } );
    auto difficultyWidget = new ObjectDifficultyWidget{ngcObject, astroGroup.telescope, bestAltitude.coordinates.altitude.degrees() };
    info->setCondition("have-difficulty", difficultyWidget->hasDifficulty() );
    info->bindWidget("difficulty", difficultyWidget );
  }
  info->setCondition("has-catalogues-descriptions", ngcObject->descriptions().size() > 0);
  if(info->conditionValue("has-catalogues-descriptions")) {
    info->bindWidget("catalogues-description", new CataloguesDescriptionWidget{ngcObject->descriptions()});
  }
  
  auto astroSessionObject = astroGroup.astroSessionObject;
  if(!astroSessionObject)
    astroSessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(ngcObject.id()).where("astro_session_id = ?").bind(astroSession.id());

  info->setCondition("has-custom-description", astroSessionObject && astroSessionObject->description().size() > 0);
  if(info->conditionValue("has-custom-description")) {
    info->bindWidget("custom-description",new WHTMLTxt{astroSessionObject->description()});
  }

  info->setCondition("has-report", astroSessionObject && astroSessionObject->observed() && astroSessionObject->report());
  if(info->conditionValue("has-report")) {
    info->bindWidget("report", new WHTMLTxt{*astroSessionObject->report(), Utils::EncodeNewLines });
  }

  auto actionButtons = this->actionButtons;
  auto astroSessionObjectsWithDescription = session.find<AstroSessionObject>().where("description is not null").where("length(description) > 0").where("objects_id = ?").bind(ngcObject.id()).where("astro_session_id <> ?").bind(astroSession.id()).resultList();
  if(astroSessionObjectsWithDescription.size() > 0) {
    WTemplate *rowsTemplate = new WTemplate();
    stringstream rows;
    for(int index = 0; index<astroSessionObjectsWithDescription.size(); index++)
      rows << format(R"(<tr class='other-desc' style='display: none;'><td><small class="astroobject_text">${object-description-header-%d}:</small></td><td><small class="astroobject_text">${object-description-%d}</small></td></tr>)") % index % index;

    rowsTemplate->setTemplateText(rows.str(), XHTMLUnsafeText);
    actionButtons.push_back(WW<WPushButton>(WString::tr("btn_other_users_descriptions")).addCss("btn-xs").onClick([=](WMouseEvent){
      wApp->doJavaScript(format(R"($('#%s >table > tbody >.other-desc').toggle();)") % info->id() );
    }));
    int index = 0;
    for(auto description: astroSessionObjectsWithDescription) {
      rowsTemplate->bindWidget(format("object-description-header-%d") % index, new WHTMLTxt{WString("{1}").arg(description->astroSession()->user()->loginName())});
      rowsTemplate->bindWidget(format("object-description-%d") % index++, new WText{WString("<b>{1}, {2}</b><br />{3}")
        .arg(WString::fromUTF8(description->astroSession()->name()))
        .arg(description->astroSession()->wDateWhen().toString("dddd, d MMM, yyyy"))
        .arg( Utils::htmlEncode(WString::fromUTF8(description->description()) ) ) } );
    }
    info->bindWidget("other-descriptions", rowsTemplate);
    } else {
    info->bindEmpty("other-descriptions");
  }

  info->setCondition("have-actions", actionButtons.size() > 0);
  if(info->conditionValue("have-actions")) {
    actionsToolbar = WW<WContainerWidget>().css("btn-group");
    for(auto btn: actionButtons)
      actionsToolbar->addWidget(btn);
    
    info->bindWidget("actions", actionsToolbar);
  }
  row->addWidget(WW<WContainerWidget>().css("col-xs-5 col-lg-5").add(dssPage));
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

