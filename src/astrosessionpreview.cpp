/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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
#include "Models"
#include "private/astrosessionpreview_p.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/d_ptr_implementation.h"
#include "skyplanner.h"
#include <Wt/Utils>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WTimer>
#include <Wt/WToolBar>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WLabel>
#include "widgets/astroobjectstable.h"
#include <mutex>
#include "widgets/astroobjectwidget.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroSessionPreview::Private::Private(const AstroGroup& astroGroup, Session& session, AstroSessionPreview* q)
  : astroGroup(astroGroup), session(session), q(q)
{
}

AstroSessionPreview::AstroSessionPreview(const AstroGroup& astroGroup, Session& session, Wt::WContainerWidget* parent)
  : WCompositeWidget(parent), d(astroGroup, session, this)
{
bool isReport = false; // TODO
WStackedWidget *sessionStacked; // TODO
WContainerWidget *sessionContainer; // TODO

  spLog("notice") << "Switching to preview version..";
  WContainerWidget *sessionPreviewContainer = WW<WContainerWidget>().css("astroobjects-list");
  setImplementation(sessionPreviewContainer);
  sessionPreviewContainer->setStyleClass("astroobjects-list");
  sessionPreviewContainer->addWidget(WW<WText>(WString("<h3>{1}, {2}</h3>").arg(Utils::htmlEncode(WString::fromUTF8(d->astroGroup.astroSession()->name()))).arg(d->astroGroup.astroSession()->wDateWhen().toString("dddd d MMMM yyyy") )).css("text-center") );
  WPushButton *printButton = WW<WPushButton>(WString::tr("buttons_print")).css("btn-info btn-sm");
  printButton->clicked().connect([=](WMouseEvent){
    wApp->doJavaScript("window.print();", false);
    printButton->disable();
    WTimer::singleShot(500, [=](WMouseEvent) {
      printButton->enable();
    });
  });
  WPushButton *backButton = WW<WPushButton>(WString::tr("preview_back_to_astrosessiontab")).css("btn-warning btn-sm").onClick([=](WMouseEvent){
    sessionStacked->setCurrentWidget(sessionContainer);
    // populate();
    // TODO
    sessionPreviewContainer->clear();
  });
  WPushButton *invertAllButton = WW<WPushButton>(WString::tr("buttons_invert_all")).css("btn-sm");
  WToolBar *toolbar = WW<WToolBar>().addCss("hidden-print pull-right").addButton(backButton).addButton(invertAllButton).addButton(printButton);
  sessionPreviewContainer->addWidget(toolbar);

  WContainerWidget *infoWidget = WW<WContainerWidget>().css("astroobjects-info-widget");
  // TODO
  // updatePositionDetails(infoWidget, false);
  sessionPreviewContainer->addWidget(infoWidget);

  if(!isReport) {
    AstroObjectsTable *planetsTable = new AstroObjectsTable(session, {}, false, {}, {AstroObjectsTable::Names, AstroObjectsTable::AR, AstroObjectsTable::DEC, AstroObjectsTable::Constellation, AstroObjectsTable::Magnitude, AstroObjectsTable::AngularSize, AstroObjectsTable::TransitTime, AstroObjectsTable::MaxAltitude});
    planetsTable->addStyleClass("planets-table");
    planetsTable->setResponsive(false);
    WPanel *planetsPanel = new WPanel;
    planetsPanel->setTitle(WString::tr("astrosessiontab_planets_panel"));
    planetsPanel->setCollapsible(true);
    planetsPanel->titleBarWidget()->addStyleClass("hidden-print");
    planetsPanel->setCentralWidget(WW<WContainerWidget>().add(WW<WText>(WString("<h5>{1}</h5>").arg(WString::tr("astrosessiontab_planets_panel"))).css("visible-print") ).add(planetsTable));
    sessionPreviewContainer->addWidget(planetsPanel);
    // TODO
    // populatePlanets(planetsTable);
  } else {
    WContainerWidget *reportContainer = WW<WContainerWidget>();
    sessionPreviewContainer->addWidget(reportContainer);
    auto displayReport = [=] {
      reportContainer->clear();
      if(d->astroGroup.astroSession()->report()) {
	WTemplate *report = WW<WTemplate>(R"(
	  <dl class="dl-horizontal">
	    <dt>${report-label}</dt>
	    <dd>${report}</dd>
	  </dl>
	)").css("well");
	report->bindString("report-label", WString::tr("report-label"));
	report->bindString("report", Utils::htmlEncode(WString::fromUTF8(*d->astroGroup.astroSession()->report()), Utils::EncodeNewLines));
	reportContainer->addWidget(report);
      }
    };
    displayReport();
    toolbar->addButton(WW<WPushButton>(WString::tr("astrosessiontab_set_report")).css("btn-primary btn-sm hidden-pront").onClick([=](WMouseEvent){
      // TODO
      // setDescriptionDialog(SetDescription::report(d->astroGroup.astroSession(), displayReport, "astrosessiontab_set_report"));
    }));
  }
  sessionPreviewContainer->addWidget(WW<WText>(WString::tr("dss-embed-menu-info-message")).css("hidden-print"));

  shared_ptr<mutex> downloadImagesMutex(new mutex);
  Dbo::Transaction t(session);

  typedef pair<AstroSessionObjectPtr, Ephemeris::BestAltitude> AstroSessionObjectElement;
  vector<AstroSessionObjectElement> sessionObjects;
  // TODO: filter this section too?
  {
    Ephemeris ephemeris({d->astroGroup.astroSession()->position().latitude, d->astroGroup.astroSession()->position().longitude}, d->astroGroup.timezone);
    AstroSessionObject::generateEphemeris(ephemeris, d->astroGroup.astroSession(), d->astroGroup.timezone, t);
    auto query = session.query<AstroSessionObjectPtr>("select a from astro_session_object a inner join objects on a.objects_id = objects.id")
      .where("astro_session_id = ?").bind(d->astroGroup.astroSession().id());
    if(isReport)
      query.where("observed = ?").bind(true);
    query.orderBy("transit_time ASC, ra asc, dec asc, constellation_abbrev asc");


    auto sessionObjectsDbCollection = query.resultList();
    transform(begin(sessionObjectsDbCollection), end(sessionObjectsDbCollection), back_inserter(sessionObjects), [=,&ephemeris,&t](const AstroSessionObjectPtr &o){
      return AstroSessionObjectElement{o, o->bestAltitude(ephemeris, d->astroGroup.timezone)};
    });
  }
  shared_ptr<set<AstroObjectWidget*>> astroObjectWidgets(new set<AstroObjectWidget*>());
  AstroObjectWidget *astroObjectWidget = nullptr;
  for(auto objectelement: sessionObjects) {
    WPushButton *hideButton = WW<WPushButton>(WString::tr("buttons_hide")).css("btn-xs btn-warning");
    WPushButton *collapseButton = WW<WPushButton>(WString::tr("buttons_collapse")).css("btn-xs");
    WPushButton *hideDSSButton = WW<WPushButton>(WString::tr("buttons_hide_dss")).css("btn-xs");
    WPushButton *deleteButton = WW<WPushButton>(WString::tr("astroobject_remove_from_session")).css("btn-xs btn-danger");
    WPushButton *editDescriptionButton = WW<WPushButton>(WString::tr("astroobject_actions_edit_description")).css("btn-xs");
    astroObjectWidget = new AstroObjectWidget({objectelement.first, d->astroGroup.telescope, d->astroGroup.timezone}, session, downloadImagesMutex, { editDescriptionButton, collapseButton, hideDSSButton, hideButton, deleteButton });
    astroObjectWidget->addStyleClass("astroobject-list-item");
    hideButton->clicked().connect([=](WMouseEvent){astroObjectWidgets->erase(astroObjectWidget); delete astroObjectWidget; });
    // TODO
//    deleteButton->clicked().connect([=](WMouseEvent){ astroObjectWidgets->erase(astroObjectWidget); remove(objectelement.first, [=] { delete astroObjectWidget; }); } );
    hideDSSButton->clicked().connect([=](WMouseEvent){ astroObjectWidget->setDSSVisible(!astroObjectWidget->isDSSVisible()); hideDSSButton->setText(WString::tr( astroObjectWidget->isDSSVisible() ? "buttons_hide_dss" : "buttons_show_dss" ));  });
    collapseButton->clicked().connect([=](WMouseEvent) { astroObjectWidget->setCollapsed(!astroObjectWidget->isCollapsed()); });
    editDescriptionButton->clicked().connect([=](WMouseEvent) {
      Dbo::Transaction t(d->session);
      // TODO
      //setDescriptionDialog(SetDescription::description( objectelement.first, [=] { astroObjectWidget->reload(); }));
    });
    astroObjectWidgets->insert(astroObjectWidget);
    sessionPreviewContainer->addWidget(astroObjectWidget);
  }
  if(astroObjectWidget)
    astroObjectWidget->addStyleClass("astroobject-last-list-item");
  invertAllButton->clicked().connect([=](WMouseEvent){ for(auto a: *astroObjectWidgets) a->toggleInvert(); } );
  sessionStacked->setCurrentWidget(sessionPreviewContainer);
}

AstroSessionPreview::~AstroSessionPreview()
{
}
