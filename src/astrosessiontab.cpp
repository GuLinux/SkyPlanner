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
#include <Wt/WLocalDateTime>
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
  wApp->log("notice") << __PRETTY_FUNCTION__ << ", astroSession: " << astroSession.id() << ", id=" << id();
  d->reload();
}


void AstroSessionTab::Private::reload()
{
  q->clear();
  WContainerWidget *actionsContainer = WW<WContainerWidget>().setMargin(10);
  q->addWidget(actionsContainer);
  pastObservation = astroSession->wDateWhen() < WDateTime::currentDateTime();
  
  WContainerWidget *sessionInfo = WW<WContainerWidget>();
  sessionInfo->addWidget(new WText{WLocalDateTime(astroSession->wDateWhen().date(), astroSession->wDateWhen().time())
    .toString("dddd dd MMMM yyyy")});


  sessionInfo->addWidget(positionDetails = WW<WContainerWidget>());

  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);

  auto locationPanel = addPanel(WString::tr("position_title"), placeWidget ); 
  addPanel(WString::tr("astrosessiontab_information_panel"), sessionInfo, true);
  if(astroSession->position()) {
    placeWidget->mapReady().connect([=](_n6){ WTimer::singleShot(1500, [=](WMouseEvent){
        locationPanel->collapse();
      });
    });
  }
  SelectObjectsWidget *addObjectsTabWidget = new SelectObjectsWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    addObjectsTabWidget->populateFor(selectedTelescope);
    updatePositionDetails();
  });
  addPanel(WString::tr("astrosessiontab_add_observable_object"), addObjectsTabWidget, true);
  addObjectsTabWidget->objectsListChanged().connect([=](_n6){populate(); });
  q->addWidget(new WText{WString("<h3>{1}</h3>").arg(WString::tr("astrosessiontab_objects_title"))});

  q->addWidget(objectsTable = WW<WTable>().addCss("table table-striped table-hover"));
  objectsTable->setHeaderCount(1);
  
  Dbo::Transaction t(session);
  WToolBar *sessionActions = WW<WToolBar>();
    
  sessionActions->addButton(WW<WPushButton>(WString::tr("astrosessiontab_change_name_or_date")).css("btn btn-small").onClick([=](WMouseEvent){
    WDialog *changeNameOrDateDialog = new WDialog(WString::tr("astrosessiontab_change_name_or_date"));
    WLineEdit *sessionName = WW<WLineEdit>(astroSession->name()).css("input-block-level");
    WDateEdit *sessionDate = WW<WDateEdit>().css("input-block-level");
    sessionDate->setDate(astroSession->wDateWhen().date());
    changeNameOrDateDialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).css("btn btn-primary").onClick([=](WMouseEvent){
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
  sessionActions->addButton(WW<WPushButton>(WString::tr("astrosessiontab_printable_version")).css("btn btn-info btn-small").onClick( [=](WMouseEvent){ printableVersion(); } ));
  actionsContainer->addWidget(sessionActions);
  auto telescopes = session.user()->telescopes();
  if(telescopes.size() > 0) {
    WComboBox *telescopeCombo = new WComboBox;
    telescopeCombo->setWidth(350);
    WLabel *telescopeComboLabel = WW<WLabel>(WString::tr("astrosessiontab__telescope_label")).setMargin(10);
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
    actionsContainer->addWidget(WW<WText>(WString::tr("astrosessiontab_no_telescopes_message")).css("pull-right"));
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
  WDialog *printableDialog = new WDialog(WString::tr("astrosessiontab_printable_version"));
  WPushButton *okButton;
  printableDialog->footer()->addWidget(okButton = WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).css("btn btn-primary").onClick([=](WMouseEvent){ printableDialog->accept(); }));
  printableDialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Cancel")).css("btn btn-danger").onClick([=](WMouseEvent){ printableDialog->reject(); }));
  auto printableResource = new PrintableAstroSessionResource(astroSession, session, q);
#ifdef DISABLE_LIBHARU
#define PDF_INDEX -1
  printableResource->setReportType(PrintableAstroSessionResource::HTML);
#else
#define PDF_INDEX 0
  printableResource->setReportType(PrintableAstroSessionResource::PDF);
#endif
  okButton->setLink(printableResource);
  okButton->setLinkTarget(TargetNewWindow);
  printableDialog->contents()->addWidget(new WLabel(WString::tr("printable_version_dialog_spacing_between_objects")));
  printableDialog->contents()->addWidget(new WBreak);
  WSlider *emptyRowsSlider = new WSlider();
  emptyRowsSlider->setWidth(500);
  emptyRowsSlider->setMaximum(10);
  emptyRowsSlider->valueChanged().connect([=](int v, _n5){printableResource->setRowsSpacing(v); });
  printableDialog->contents()->addWidget(emptyRowsSlider);
  printableDialog->contents()->addWidget(new WBreak);
  

  
  WSlider *fontScalingSlider = new WSlider();
  WComboBox *formatCombo = new WComboBox();
#ifdef DISABLE_LIBHARU
  formatCombo->addItem("PDF");
#endif
  formatCombo->addItem("HTML");
  formatCombo->activated().connect([=](int r, _n5){
    printableResource->setReportType(r==PDF_INDEX ? PrintableAstroSessionResource::PDF : PrintableAstroSessionResource::HTML);
    fontScalingSlider->setEnabled(r==PDF_INDEX);
  });
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(new WLabel{WString::tr("astrosessiontab_printable_version_dialog_export_as")}).add(formatCombo).add(new WBreak));
  fontScalingSlider->setWidth(500);
  fontScalingSlider->setMaximum(40);
  fontScalingSlider->setValue(20);
  WText *fontScalingValue = WW<WText>("100%").css("pull-right").setMargin(15, Top);
  fontScalingSlider->valueChanged().connect([=](int v, _n5){
    double value = 2. / 40. * static_cast<double>(v);
    printableResource->setFontScale( value );
    fontScalingValue->setText(format("%d%%") % static_cast<int>(value*100));
  });
  printableDialog->contents()->addWidget(new WLabel(WString::tr("printable_version_dialog_fonts_size")));
  printableDialog->contents()->addWidget(new WBreak);
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(fontScalingSlider).add(fontScalingValue));
  printableDialog->contents()->addWidget(new WBreak);
  
  Dbo::Transaction t(session);
  auto telescopes = session.user()->telescopes();
  switch(telescopes.size()) {
    case 0:
      printableDialog->contents()->addWidget(new WText{WString::tr("printable_version_dialog_add_telescope_suggestion")});
      break;
    case 1:
      printableResource->setTelescope(telescopes.front());
      printableDialog->contents()->addWidget(new WText{WString(WString::tr("printable_version_dialog_using_telescope")).arg(telescopes.front()->name()) });
      break;
    default:
      printableResource->setTelescope(telescopes.front());
      printableDialog->contents()->addWidget(new WLabel{WString::tr("printable_version_dialog_telescope_combo_label")});
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
    positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_moon_phase")).arg(static_cast<int>(lunarPhase.illuminated_fraction * 100 ))));
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
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_sun_info"))
    .arg(formatTime(sun.rise))
    .arg(formatTime(sun.set))
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_moon_info"))
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
  objectsTable->elementAt(0,0)->addWidget(new WText{WString::tr("object_column_names")});
  objectsTable->elementAt(0,1)->addWidget(new WText{WString::tr("object_column_type")});
  objectsTable->elementAt(0,2)->addWidget(new WText{WString::tr("object_column_ar")});
  objectsTable->elementAt(0,3)->addWidget(new WText{WString::tr("object_column_dec")});
  objectsTable->elementAt(0,4)->addWidget(new WText{WString::tr("object_column_constellation")});
  objectsTable->elementAt(0,5)->addWidget(new WText{WString::tr("object_column_angular_size")});
  objectsTable->elementAt(0,6)->addWidget(new WText{WString::tr("object_column_magnitude")});
  objectsTable->elementAt(0,7)->addWidget(new WText{WString::tr("object_column_highest_time")});
  objectsTable->elementAt(0,8)->addWidget(new WText{WString::tr("object_column_max_altitude")});
  objectsTable->elementAt(0,9)->addWidget(new WText{WString::tr("object_column_difficulty")});
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
    row->elementAt(4)->addWidget(new WText{ WString::fromUTF8(ConstellationFinder::getName(sessionObject->coordinates()).name) });
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
      .add(new WLabel{WString::tr("object_notes")})
      .add(descriptionTextArea)
      .add(WW<WPushButton>(WString::tr("buttons_save")).css("btn btn-mini btn-primary pull-right").onClick([=](WMouseEvent){
        Dbo::Transaction t(session);
        sessionObject.modify()->setDescription(descriptionTextArea->text().toUTF8());
	AstroPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("notification_description_saved"), AstroPlanner::Success, 5);
      }));
    descriptionCell->addWidget(descriptionContainer);
    WToolBar *actions = new WToolBar;
    row->elementAt(10)->addWidget(actions);
    actions->addButton(WW<WPushButton>(WString::tr("description")).css("btn btn-mini").onClick([=](WMouseEvent){
      descriptionCell->setHidden(!descriptionCell->isHidden());
    }));
    actions->addButton(WW<WPushButton>(WString::tr("buttons_remove")).css("btn btn-danger btn-mini").onClick([=](WMouseEvent){
      WMessageBox *confirmation = new WMessageBox(WString::tr("messagebox_confirm_removal_title"), WString::tr("messagebox_confirm_removal_message"), Wt::Question, Wt::Ok | Wt::Cancel);
      confirmation->buttonClicked().connect([=](StandardButton b, _n5) {
        if(b != Wt::Ok) {
          confirmation->reject();
          return;
        }
        confirmation->accept();
        Dbo::Transaction t(session);
        astroSession.modify()->astroSessionObjects().erase(sessionObject);
        Dbo::ptr<AstroSessionObject> o = sessionObject;
        o.remove();
        t.commit();
        populate();
      });
      confirmation->show();
    }));

    if(pastObservation) {
      WPushButton *observedToggleButton = WW<WPushButton>().css("btn btn-mini");
      observedToggleButton->setTextFormat(XHTMLUnsafeText);
      auto setObservedButtonStyle = [=]( const Dbo::ptr<AstroSessionObject> &o) {
        observedToggleButton->setText(o->observed() ? WString::tr("astrosessiontab_object_observed") : WString::tr("astrosessiontab_object_not_observed"));
        observedToggleButton->addStyleClass(o->observed() ? "btn-success" : "btn-inverse");
        observedToggleButton->removeStyleClass(!o->observed() ? "btn-success" : "btn-inverse");
      };
      observedToggleButton->clicked().connect([=](WMouseEvent) {
        Dbo::Transaction t(session);
        Dbo::ptr<AstroSessionObject> o = session.find<AstroSessionObject>().where("id = ?").bind(sessionObject.id());
        o.modify()->setObserved(!sessionObject->observed());
        t.commit();
        setObservedButtonStyle(o);
      });
      setObservedButtonStyle(sessionObject);
      actions->addButton(observedToggleButton);
    }
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

