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
#include <Wt/WTextArea>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WDateEdit>
#include <Wt/WLineEdit>
#include <Wt/WToolBar>
#include <Wt/WTemplate>
#include "constellationfinder.h"
#include "printableastrosessionresource.h"
#include <Wt/WSlider>
#include "astroplanner.h"


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
  d->reload();
}


void AstroSessionTab::Private::reload()
{
  q->clear();
  WContainerWidget *actionsContainer = WW<WContainerWidget>().setMargin(10);
  q->addWidget(actionsContainer);
  
  WContainerWidget *sessionInfo = WW<WContainerWidget>();
  sessionInfo->addWidget(new WText{astroSession->wDateWhen().date().toString("dddd dd MMMM yyyy")});
  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);
  SelectObjectsWidget *addObjectsTabWidget = new SelectObjectsWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    addObjectsTabWidget->populateFor(selectedTelescope);
    updatePositionDetails();
  });
  sessionInfo->addWidget(positionDetails = WW<WContainerWidget>());
  auto locationPanel = addPanel("Position", placeWidget ); 
  addPanel("Information", sessionInfo);
  if(astroSession->position()) {
    placeWidget->mapReady().connect([=](_n6){ WTimer::singleShot(1500, [=](WMouseEvent){ locationPanel->collapse(); }); });
  }
  addPanel("Add Observable Object", addObjectsTabWidget, true);

  addObjectsTabWidget->objectsListChanged().connect([=](_n6){populate(); });
  q->addWidget(new WText{"<h3>Objects</h3>"});

  q->addWidget(objectsTable = WW<WTable>().addCss("table table-striped table-hover"));
  objectsTable->setHeaderCount(1);
  
  Dbo::Transaction t(session);
  WToolBar *sessionActions = WW<WToolBar>();
    
  sessionActions->addButton(WW<WPushButton>("Change name or date").css("btn").onClick([=](WMouseEvent){
    WDialog *changeNameOrDateDialog = new WDialog("Change name or date");
    WLineEdit *sessionName = WW<WLineEdit>(astroSession->name()).css("input-block-level");
    WDateEdit *sessionDate = WW<WDateEdit>().css("input-block-level");
    sessionDate->setDate(astroSession->wDateWhen().date());
    changeNameOrDateDialog->footer()->addWidget(WW<WPushButton>("Ok").css("btn btn-primary").onClick([=](WMouseEvent){
      Dbo::Transaction t(session);
      astroSession.modify()->setName(sessionName->text().toUTF8());
      astroSession.modify()->setDateTime(WDateTime{sessionDate->date()});
      changeNameOrDateDialog->accept();
      nameChanged.emit(astroSession->name());
      reload();
    }));
    WTemplate *form = new WTemplate("<form><fieldset><label>Name</label>${sessionName}<label>Date</label>${sessionDate}</fieldset></form>");
    form->bindWidget("sessionName", sessionName);
    form->bindWidget("sessionDate", sessionDate);
    changeNameOrDateDialog->contents()->addWidget(form);
    changeNameOrDateDialog->show();
  }));
  sessionActions->addButton(WW<WPushButton>("Printable Version").css("btn btn-info").onClick( [=](WMouseEvent){ printableVersion(); } ));
  actionsContainer->addWidget(sessionActions);
  auto telescopes = session.user()->telescopes();
  if(telescopes.size() > 0) {
    WComboBox *telescopeCombo = new WComboBox;
    telescopeCombo->setWidth(350);
    WLabel *telescopeComboLabel = WW<WLabel>("Telescope: ").setMargin(10);
    telescopeComboLabel->setBuddy(telescopeCombo);

    actionsContainer->addWidget(WW<WContainerWidget>().css("form-inline pull-right").add(telescopeComboLabel).add(telescopeCombo));
    WStandardItemModel *model = new WStandardItemModel(q);
    for(auto telescope: telescopes) {
      if(!selectedTelescope)
        selectedTelescope = telescope;
      WStandardItem *item = new WStandardItem(telescope->name());
      item->setData(telescope);
      model->appendRow(item);
    }
    telescopeCombo->setModel(model);
    
    telescopeCombo->activated().connect([=](int index, _n5){
      selectedTelescope = boost::any_cast<Dbo::ptr<Telescope>>(model->item(index)->data());
      populate();
      addObjectsTabWidget->populateFor(selectedTelescope);
    });
  } else {
    actionsContainer->addWidget(WW<WText>("Add one or more telescopes in the \"My Telescopes\" section to see personalized suggestions and data here.").css("pull-right"));
  }
  
  populate();
  updatePositionDetails();
  WTimer::singleShot(200, [=](WMouseEvent) {
    addObjectsTabWidget->populateFor(selectedTelescope);
  });
}


Wt::Signal<std::string> &AstroSessionTab::nameChanged() const
{
  return d->nameChanged;
}


void AstroSessionTab::Private::printableVersion()
{
  WDialog *printableDialog = new WDialog("Printable Version");
  WPushButton *okButton;
  printableDialog->footer()->addWidget(okButton = WW<WPushButton>("Ok").css("btn btn-primary").onClick([=](WMouseEvent){ printableDialog->accept(); }));
  printableDialog->footer()->addWidget(WW<WPushButton>("Cancel").css("btn btn-danger").onClick([=](WMouseEvent){ printableDialog->reject(); }));
  auto printableResource = new PrintableAstroSessionResource(astroSession, session, q);
  printableResource->setReportType(PrintableAstroSessionResource::PDF);
  okButton->setLink(printableResource);
  okButton->setLinkTarget(TargetNewWindow);
  printableDialog->contents()->addWidget(new WLabel("Spacing between objects rows"));
  printableDialog->contents()->addWidget(new WBreak);
  WSlider *emptyRowsSlider = new WSlider();
  emptyRowsSlider->setWidth(500);
  emptyRowsSlider->setMaximum(10);
  emptyRowsSlider->valueChanged().connect([=](int v, _n5){printableResource->setRowsSpacing(v); });
  printableDialog->contents()->addWidget(emptyRowsSlider);
  printableDialog->contents()->addWidget(new WBreak);
  

  
  WSlider *fontScalingSlider = new WSlider();
  WComboBox *formatCombo = new WComboBox();
  formatCombo->addItem("PDF");
  formatCombo->addItem("HTML");
  formatCombo->activated().connect([=](int r, _n5){
    printableResource->setReportType(r==0 ? PrintableAstroSessionResource::PDF : PrintableAstroSessionResource::HTML); 
    fontScalingSlider->setEnabled(r==0);
  });
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(new WLabel{"Export as..."}).add(formatCombo).add(new WBreak));
  fontScalingSlider->setWidth(500);
  fontScalingSlider->setMaximum(40);
  fontScalingSlider->setValue(20);
  WText *fontScalingValue = WW<WText>("100%").css("pull-right").setMargin(15, Top);
  fontScalingSlider->valueChanged().connect([=](int v, _n5){
    double value = 2. / 40. * static_cast<double>(v);
    printableResource->setFontScale( value );
    fontScalingValue->setText(format("%d%%") % static_cast<int>(value*100));
  });
  printableDialog->contents()->addWidget(new WLabel("Fonts size (PDF Only)"));
  printableDialog->contents()->addWidget(new WBreak);
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(fontScalingSlider).add(fontScalingValue));
  printableDialog->contents()->addWidget(new WBreak);
  
  Dbo::Transaction t(session);
  auto telescopes = session.user()->telescopes();
  switch(telescopes.size()) {
    case 0:
      printableDialog->contents()->addWidget(new WText{"Please add one or more telescope to see suggestions"});
      break;
    case 1:
      printableResource->setTelescope(telescopes.front());
      printableDialog->contents()->addWidget(new WText{WString("Using Telescope {1}").arg(telescopes.front()->name()) });
      break;
    default:
      printableResource->setTelescope(telescopes.front());
      printableDialog->contents()->addWidget(new WLabel{"Telescope: "});
      WComboBox *telescopesCombo = new WComboBox(printableDialog->contents());
      WStandardItemModel *telescopesModel = new WStandardItemModel(printableDialog);
      telescopesCombo->setModel(telescopesModel);
      for(auto telescope: telescopes) {
	WStandardItem *item = new WStandardItem(telescope->name());
	item->setData(telescope);
	telescopesModel->appendRow(item);
      }
      telescopesCombo->activated().connect([=](int i, _n5) {
	printableResource->setTelescope(boost::any_cast<Dbo::ptr<Telescope>>(telescopesModel->item(i)->data()));
      });
  }
  printableDialog->show();
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

  auto formatTime = [](const boost::posix_time::ptime &t) { return (format("%02d:%02d") % t.time_of_day().hours() % t.time_of_day().minutes()).str(); };
  positionDetails->addWidget(new WText(WString("Sun: rising at {1}, setting at {2}")
    .arg(formatTime(sun.rise))
    .arg(formatTime(sun.set))
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString("Moon: rising at {1}, setting at {2}")
    .arg(formatTime(moon.rise))
    .arg(formatTime(moon.set))
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
  objectsTable->elementAt(0,1)->addWidget(new WText{"Type"});
  objectsTable->elementAt(0,2)->addWidget(new WText{"AR"});
  objectsTable->elementAt(0,3)->addWidget(new WText{"DEC"});
  objectsTable->elementAt(0,4)->addWidget(new WText{"Constellation"});
  objectsTable->elementAt(0,5)->addWidget(new WText{"Angular Size"});
  objectsTable->elementAt(0,6)->addWidget(new WText{"Magnitude"});
  objectsTable->elementAt(0,7)->addWidget(new WText{"Highest Time"});
  objectsTable->elementAt(0,8)->addWidget(new WText{"Max Altitude"});
  objectsTable->elementAt(0,9)->addWidget(new WText{"Difficulty"});
  Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude});
  boost::posix_time::ptime sessionTimeStart = ephemeris.sun(astroSession->when()).set;
  boost::posix_time::ptime sessionTimeEnd = ephemeris.sun(astroSession->when() + boost::posix_time::hours(24)).rise;
  Dbo::Transaction t(session);
  
  // TODO: optimize
  auto sessionObjectsDbCollection = astroSession->astroSessionObjects();
  typedef pair<dbo::ptr<AstroSessionObject>, Ephemeris::BestAltitude> AstroSessionObjectElement;
  vector<AstroSessionObjectElement> sessionObjects;
  transform(begin(sessionObjectsDbCollection), end(sessionObjectsDbCollection), back_inserter(sessionObjects), [&ephemeris](const dbo::ptr<AstroSessionObject> &o){
    return AstroSessionObjectElement{o, o->bestAltitude(ephemeris, -3)};
  });
  sort(begin(sessionObjects), end(sessionObjects), [&](const AstroSessionObjectElement &a, const AstroSessionObjectElement &b){
    return a.second.when < b.second.when;
  });
  for(auto sessionObjectElement: sessionObjects) {
    dbo::ptr<AstroSessionObject> sessionObject = sessionObjectElement.first;
    WTableRow *row = objectsTable->insertRow(objectsTable->rowCount());
    row->elementAt(0)->addWidget(new ObjectNamesWidget{sessionObject->ngcObject(), session, astroSession});
    row->elementAt(1)->addWidget(new WText{sessionObject->ngcObject()->typeDescription() });
    row->elementAt(2)->addWidget(new WText{ Utils::htmlEncode( sessionObject->coordinates().rightAscension.printable(Angle::Hourly) ) });
    row->elementAt(3)->addWidget(new WText{ Utils::htmlEncode( WString::fromUTF8( sessionObject->coordinates().declination.printable() )) });
    row->elementAt(4)->addWidget(new WText{ ConstellationFinder::getName(sessionObject->coordinates()).name });
    row->elementAt(5)->addWidget(new WText{ Utils::htmlEncode( WString::fromUTF8( Angle::degrees(sessionObject->ngcObject()->angularSize()).printable() )) });
    row->elementAt(6)->addWidget(new WText{ format("%.1f") % sessionObject->ngcObject()->magnitude()});
    auto bestAltitude = sessionObject->bestAltitude(ephemeris, 1);
    row->elementAt(7)->addWidget(new WText{ WDateTime::fromPosixTime( bestAltitude.when).time().toString() });
    row->elementAt(8)->addWidget(new WText{ Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) });
    row->elementAt(9)->addWidget(new ObjectDifficultyWidget{sessionObject->ngcObject(), selectedTelescope, bestAltitude.coordinates.altitude.degrees() }); 
    WTableRow *descriptionRow = objectsTable->insertRow(objectsTable->rowCount());
    WTableCell *descriptionCell = descriptionRow->elementAt(0);
    descriptionCell->setHidden(true);
    descriptionCell->setColumnSpan(11);
    WTextArea *descriptionTextArea = WW<WTextArea>(sessionObject->description()).css("input-block-level");
    WContainerWidget *descriptionContainer = WW<WContainerWidget>()
      .add(new WLabel{"Your notes and description"})
      .add(descriptionTextArea)
      .add(WW<WPushButton>("Save").css("btn btn-mini btn-primary pull-right").onClick([=](WMouseEvent){
        Dbo::Transaction t(session);
        sessionObject.modify()->setDescription(descriptionTextArea->text().toUTF8());
	AstroPlanner::instance()->notification("Success", "Description saved!", AstroPlanner::Success, 5);
      }));
    descriptionCell->addWidget(descriptionContainer);
    WToolBar *actions = new WToolBar;
    row->elementAt(10)->addWidget(actions);
    actions->addButton(WW<WPushButton>("Description").css("btn btn-mini").onClick([=](WMouseEvent){
      descriptionCell->setHidden(!descriptionCell->isHidden());
    }));
    actions->addButton(WW<WPushButton>("Remove").css("btn btn-danger btn-mini").onClick([=](WMouseEvent){
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
  panel->setCentralWidget(widget);
  return panel;
}

