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
#include "utils/format.h"
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
#include "aaplus/AADate.h"
#include "aaplus/AAMoon.h"
#include "aaplus/AAMoonIlluminatedFraction.h"
#include "aaplus/AAElliptical.h"
#include "aaplus/AARiseTransitSet.h"
#include "ephemeris.h"
#include "types.h"
#include "selectobjectswidget.h"
#include "widgets/objectdifficultywidget.h"
#include "widgets/objectnameswidget.h"
#include <Wt/Utils>
#include <Wt/WTimer>
#include <boost/format.hpp>
#include <Wt/WImage>
#include <Wt/WLabel>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include "constellationfinder.h"

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
  WContainerWidget *telescopeComboContainer = WW<WContainerWidget>().setMargin(10);
  addWidget(telescopeComboContainer);
  
  
  WContainerWidget *sessionInfo = WW<WContainerWidget>();
  sessionInfo->addWidget(new WText(astroSession->wDateWhen().date().toString("dddd dd MMMM yyyy")));
  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    d->updatePositionDetails();
  });
  sessionInfo->addWidget(d->positionDetails = WW<WContainerWidget>());
  auto locationPanel = d->addPanel("Location", placeWidget ); 
  d->addPanel("Information", sessionInfo);
  if(astroSession->position()) {
    placeWidget->mapReady().connect([=](_n6){ WTimer::singleShot(1500, [=](WMouseEvent){ locationPanel->collapse(); }); });
  }
  SelectObjectsWidget *addObjectsTabWidget = new SelectObjectsWidget(astroSession, session);
  d->addPanel("Add Observable Object", addObjectsTabWidget, true);

  addObjectsTabWidget->objectsListChanged().connect([=](_n6){d->populate(); });
  addWidget(new WText{"<h3>Objects</h3>"});

  addWidget(d->objectsTable = WW<WTable>().addCss("table table-striped table-hover"));
  d->objectsTable->setHeaderCount(1);
  
  Dbo::Transaction t(d->session);
  auto telescopes = d->session.user()->telescopes();
  if(telescopes.size() > 0) {
    WComboBox *telescopeCombo = new WComboBox;
    telescopeCombo->setWidth(350);
    WLabel *telescopeComboLabel = new WLabel("Telescope: ");;
    telescopeComboLabel->setBuddy(telescopeCombo);
    telescopeComboContainer->addStyleClass("form-inline");
    telescopeComboContainer->addWidget(telescopeComboLabel);
    telescopeComboContainer->addWidget(telescopeCombo);
    WStandardItemModel *model = new WStandardItemModel(this);
    for(auto telescope: telescopes) {
      if(!d->selectedTelescope)
        d->selectedTelescope = telescope;
      WStandardItem *item = new WStandardItem(telescope->name());
      item->setData(telescope);
      model->appendRow(item);
    }
    telescopeCombo->setModel(model);
    
    telescopeCombo->activated().connect([=](int index, _n5){
      d->selectedTelescope = boost::any_cast<Dbo::ptr<Telescope>>(model->item(index)->data());
      d->populate();
      addObjectsTabWidget->populateFor(d->selectedTelescope);
    });
  } else {
    telescopeComboContainer->addWidget(new WText{"Add one or more telescopes in the \"My Telescopes\" section to see personalized suggestions and data here."});
  }
  
  d->populate();
  d->updatePositionDetails();
  WTimer::singleShot(200, [=](WMouseEvent) {
    addObjectsTabWidget->populateFor(d->selectedTelescope);
  });
}

void AstroSessionTab::Private::updatePositionDetails()
{
  Dbo::Transaction t(session);
  astroSession.reread();
  positionDetails->clear();
  auto addMoonPhaseDetails = [=](const Ephemeris &ephemeris) {
    Ephemeris::LunarPhase lunarPhase = ephemeris.moonPhase(astroSession->when());
    positionDetails->addWidget(new WText(WString("Moon Phase: {1}%").arg(static_cast<int>(lunarPhase.illuminated_fraction * 100 ))));
    positionDetails->addWidget(new WBreak);
  };
  if(!astroSession->position()) {
    addMoonPhaseDetails(Ephemeris{{}});
    return;
  }
//   forecast.fetch(astroSession->position().longitude, astroSession->position().latitude);
  WDateTime when = astroSession->wDateWhen();
  Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude});
  Ephemeris::RiseTransitSet sun = ephemeris.sun(astroSession->when());
  Ephemeris::RiseTransitSet moon = ephemeris.moon(astroSession->when());

  positionDetails->addWidget(new WText(WString("Sun: rising at {1}, setting at {2}")
    .arg(WDateTime::fromPosixTime(sun.rise).toString())
    .arg(WDateTime::fromPosixTime(sun.set).toString())
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString("Moon: rising at {1}, setting at {2}")
    .arg(WDateTime::fromPosixTime(sun.rise).toString())
    .arg(WDateTime::fromPosixTime(sun.set).toString())
  ));
  positionDetails->addWidget(new WBreak);
  addMoonPhaseDetails(ephemeris);
  
  positionDetails->addWidget(new WImage(WLink{format("http://www.7timer.com/v4/bin/astro.php?lon=%f&lat=%f&lang=en&ac=0&unit=metric&tzshift=0")
    % astroSession->position().longitude.degrees() % astroSession->position().latitude.degrees()
  } ));
}


void AstroSessionTab::Private::populate()
{
  objectsTable->clear();
  objectsTable->elementAt(0,0)->addWidget(new WText{"Names"});
  objectsTable->elementAt(0,1)->addWidget(new WText{"AR"});
  objectsTable->elementAt(0,2)->addWidget(new WText{"DEC"});
  objectsTable->elementAt(0,3)->addWidget(new WText{"Constellation"});
  objectsTable->elementAt(0,4)->addWidget(new WText{"Angular Size"});
  objectsTable->elementAt(0,5)->addWidget(new WText{"Magnitude"});
  objectsTable->elementAt(0,6)->addWidget(new WText{"Type"});
  objectsTable->elementAt(0,7)->addWidget(new WText{"Highest Time"});
  objectsTable->elementAt(0,8)->addWidget(new WText{"Max Altitude"});
  objectsTable->elementAt(0,9)->addWidget(new WText{"Difficulty"});
  Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude});
  boost::posix_time::ptime sessionTimeStart = ephemeris.sun(astroSession->when()).set;
  boost::posix_time::ptime sessionTimeEnd = ephemeris.sun(astroSession->when() + boost::posix_time::hours(24)).rise;
  Dbo::Transaction t(session);
  
  // TODO: optimize
  auto sessionObjectsDbCollection = astroSession->astroSessionObjects();
  vector<dbo::ptr<AstroSessionObject>> sessionObjects(sessionObjectsDbCollection.begin(), sessionObjectsDbCollection.end());
  sort(begin(sessionObjects), end(sessionObjects), [&](const dbo::ptr<AstroSessionObject> &a, const dbo::ptr<AstroSessionObject> &b){
    return a->bestAltitude(ephemeris, -3 ).when < b->bestAltitude(ephemeris, -3).when;
  });
  for(auto sessionObject: sessionObjects) {
    WTableRow *row = objectsTable->insertRow(objectsTable->rowCount());
    sessionObject->bestAltitude(ephemeris);
    row->elementAt(0)->addWidget(new ObjectNamesWidget{sessionObject->ngcObject(), session, astroSession});
    row->elementAt(1)->addWidget(new WText{ Utils::htmlEncode( sessionObject->coordinates().rightAscension.printable(Angle::Hourly) ) });
    row->elementAt(2)->addWidget(new WText{ Utils::htmlEncode( WString::fromUTF8( sessionObject->coordinates().declination.printable() )) });
    row->elementAt(3)->addWidget(new WText{ ConstellationFinder::getName(sessionObject->coordinates()).name });
    row->elementAt(4)->addWidget(new WText{ Utils::htmlEncode( WString::fromUTF8( Angle::degrees(sessionObject->ngcObject()->angularSize()).printable() )) });
    row->elementAt(5)->addWidget(new WText{ format("%.1f") % sessionObject->ngcObject()->magnitude()});
    row->elementAt(6)->addWidget(new WText{sessionObject->ngcObject()->typeDescription() });
    auto bestAltitude = sessionObject->bestAltitude(ephemeris, 1);
    row->elementAt(7)->addWidget(new WText{ WDateTime::fromPosixTime( bestAltitude.when).time().toString() });
    row->elementAt(8)->addWidget(new WText{ Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) });
    row->elementAt(9)->addWidget(new ObjectDifficultyWidget{sessionObject, selectedTelescope, bestAltitude.coordinates.altitude.degrees() }); 
    row->elementAt(10)->addWidget(WW<WPushButton>("Remove").css("btn btn-danger").onClick([=](WMouseEvent){
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


WPanel *AstroSessionTab::Private::addPanel( const WString &title, WWidget *widget, bool collapsed, bool collapsible, WContainerWidget *container )
{
  WPanel *panel = WW<WPanel>(container ? container : q);
  panel->setTitle(title);
  panel->setCollapsible(collapsible);
  panel->setCollapsed(collapsed);
  panel->setAnimation({WAnimation::AnimationEffect::SlideInFromTop});
  panel->setCentralWidget(widget);
  return panel;
}

