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
#include "astrosessiontab.h"
#include "private/astrosessiontab_p.h"
#include "utils/d_ptr_implementation.h"
#include "Wt-Commons/wt_helpers.h"
#include "session.h"
#include "placewidget.h"
#include <Wt/WText>
#include <Wt/WComboBox>
#include <Wt/Dbo/QueryModel>
#include <Wt/WSpinBox>
#include <Wt/WTable>
#include <Wt/WPushButton>
#include <Wt/WTabWidget>
#include <Wt/WMessageBox>
#include <boost/format.hpp>
#include "AstroCpp/RiseSet.h"
#include "AstroCpp/Lunar.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroSessionTab::Private::Private(const Dbo::ptr<AstroSession>& astroSession, Session &session, AstroSessionTab* q) : astroSession(astroSession), session(session), q(q)
{
}

AstroSessionTab::~AstroSessionTab()
{
}

AstroSessionTab::AstroSessionTab(const Dbo::ptr<AstroSession>& astroSession, Session& session, WContainerWidget* parent)
    : WContainerWidget(parent), d(astroSession, session, this)
{
  WContainerWidget *sessionInfo = WW<WContainerWidget>();
  sessionInfo->addWidget(new WText(astroSession->wDateWhen().date().toString("dddd dd MMMM yyyy")));
  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    d->updatePositionDetails();
  });
  sessionInfo->addWidget(d->positionDetails = WW<WContainerWidget>());
  sessionInfo->addWidget(placeWidget);
  d->addPanel("Information", sessionInfo);
  
  WContainerWidget *addObjectByCatalogue = WW<WContainerWidget>();
  WTabWidget *addObjectsTabWidget = new WTabWidget();
  addObjectsTabWidget->addTab(addObjectByCatalogue, "Add By Catalogue Number");
  d->addPanel("Add Observable Object", addObjectsTabWidget, true);
  
  Dbo::Transaction t(d->session);
  
  WComboBox *cataloguesCombo = new WComboBox();
  WLineEdit *catalogueNumber = WW<WLineEdit>();
  WTable *resultsTable = WW<WTable>().addCss("table table-striped table-hover");

  
  // TODO: .......
  for(auto cat: vector<string>{"Messier", "NGC", "IC", "Caldwell"})
    cataloguesCombo->addItem(cat);
  catalogueNumber->setEmptyText("Catalogue Number");
  auto searchByCatalogueNumber = [=] {
    Dbo::Transaction t(d->session);
    resultsTable->clear();
    dbo::collection<dbo::ptr<NebulaDenomination>> denominations = d->session.find<NebulaDenomination>().where("catalogue = ?").where("number = ?")
      .bind(cataloguesCombo->currentText()).bind(catalogueNumber->text());
    for(auto nebula: denominations) {
      WTableRow *row = resultsTable->insertRow(resultsTable->rowCount());
      row->elementAt(0)->addWidget(new WText{nebula->catalogue()});
      row->elementAt(1)->addWidget(new WText{WString("{1}").arg(nebula->number())});
      row->elementAt(2)->addWidget(new WText{nebula->comment()});
      row->elementAt(3)->addWidget(WW<WPushButton>("Add").css("btn btn-primary").onClick([=](WMouseEvent){
        Dbo::Transaction t(d->session);
        astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(nebula->ngcObject()));
        t.commit();
        d->populate();
      }));
    }
  };
  catalogueNumber->changed().connect([=](_n1){ searchByCatalogueNumber(); });
  addObjectByCatalogue->addWidget(WW<WContainerWidget>().css("form-inline").add(cataloguesCombo).add(catalogueNumber)
    .add(WW<WPushButton>("Search").css("btn btn-primary").onClick([=](WMouseEvent){ searchByCatalogueNumber(); })));
  addObjectByCatalogue->addWidget(resultsTable);
  addWidget(d->objectsTable = WW<WTable>().addCss("table table-striped table-hover"));
  d->objectsTable->setHeaderCount(1);
  d->populate();
  d->updatePositionDetails();
}

void AstroSessionTab::Private::updatePositionDetails()
{
  Dbo::Transaction t(session);
  astroSession.reread();
  positionDetails->clear();
  if(!astroSession->position())
    return;
//   forecast.fetch(astroSession->position().longitude, astroSession->position().latitude);
  ObsInfo obsInfo(-astroSession->position().longitude, astroSession->position().latitude, 0); // TODO: timezone
  cerr << "latitude: " << obsInfo.latitude() << ", longitude: " << obsInfo.longitude() << endl;
  TimePair sunRiseSet, moonRiseSet;
  RiseSet riseSet;
  riseSet.getTimes(sunRiseSet, RiseSet::SUN, astroSession->wDateWhen().date().toJulianDay(), obsInfo);
  riseSet.getTimes(moonRiseSet, RiseSet::MOON, astroSession->wDateWhen().date().toJulianDay(), obsInfo);
  
  auto timeToString = [](double t) {
    long minutes = static_cast<long>(t * 24. * 60. + .5);
    stringstream time;
    time << (minutes / 60) << ":" << (minutes % 60);
    return time.str();
  };
  // So, so wrong... :(
  positionDetails->addWidget(new WText(WString("Sun: rising at {1}, setting at {2}").arg(timeToString(sunRiseSet.a)).arg(timeToString(sunRiseSet.b))));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString("Moon: rising at {1}, setting at {2}").arg(timeToString(moonRiseSet.a)).arg(timeToString(moonRiseSet.b))));
  positionDetails->addWidget(new WBreak);
  cerr << "Moon phase: " << Lunar(astroSession->wDateWhen().date().toJulianDay()).illuminatedFraction() << endl;
  int moonPhasePercent = Lunar(astroSession->wDateWhen().date().toJulianDay()).illuminatedFraction() * 100;
  positionDetails->addWidget(new WText(WString("Moon Phase: {1}%").arg(moonPhasePercent)));
  positionDetails->addWidget(new WBreak);
}


void AstroSessionTab::Private::populate()
{
  objectsTable->clear();
  objectsTable->elementAt(0,0)->addWidget(new WText{"Names"});
  objectsTable->elementAt(0,1)->addWidget(new WText{"AR"});
  objectsTable->elementAt(0,2)->addWidget(new WText{"DEC"});
  objectsTable->elementAt(0,3)->addWidget(new WText{"Angular Size"});
  objectsTable->elementAt(0,4)->addWidget(new WText{"Magnitude"});
  objectsTable->elementAt(0,5)->addWidget(new WText{"Type"});
  Dbo::Transaction t(session);
  for(auto sessionObject: astroSession->astroSessionObjects()) {
    WTableRow *row = objectsTable->insertRow(objectsTable->rowCount());
    auto names = sessionObject->ngcObject()->nebulae();
    stringstream namesStream;
    string separator;
    for(auto name: names) {
      namesStream << separator << name->name();
      separator = ", ";
    }
    row->elementAt(0)->addWidget(new WText(namesStream.str()));
    row->elementAt(1)->addWidget(new WText(WString("{1}").arg(sessionObject->ngcObject()->rightAscension()) ));
    row->elementAt(2)->addWidget(new WText(WString("{1}").arg(sessionObject->ngcObject()->declination()) ));
    row->elementAt(3)->addWidget(new WText(WString("{1}").arg(sessionObject->ngcObject()->angularSize()) ));
    row->elementAt(4)->addWidget(new WText(WString("{1}").arg(sessionObject->ngcObject()->magnitude()) ));
    row->elementAt(5)->addWidget(new WText(sessionObject->ngcObject()->typeDescription() ));
    row->elementAt(6)->addWidget(WW<WPushButton>("Remove").css("btn btn-danger").onClick([=](WMouseEvent){
      WMessageBox *confirmation = new WMessageBox("Confirm removal", "Are you sure?", Wt::Question, Wt::Ok | Wt::Cancel);
      confirmation->buttonClicked().connect([=](StandardButton b, _n5){
        if(b != Wt::Ok) {
          confirmation->reject();
          return;
        }
        confirmation->accept();
        Dbo::Transaction t(session);
        astroSession.modify()->astroSessionObjects().erase(sessionObject);
        t.commit();
        populate();
      });
      confirmation->show();
    }));
  }
}


void AstroSessionTab::Private::addPanel( const WString &title, WWidget *widget, bool collapsed, bool collapsible, WContainerWidget *container )
{
  WPanel *panel = WW<WPanel>(container ? container : q);
  panel->setTitle(title);
  panel->setCollapsible(collapsible);
  panel->setCollapsed(collapsed);
  panel->setAnimation({WAnimation::AnimationEffect::SlideInFromTop});
  panel->setCentralWidget(widget);
}

