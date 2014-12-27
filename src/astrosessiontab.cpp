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
#include "Models"
#include "astrosessiontab.h"
#include "private/astrosessiontab_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/format.h"
#include "utils/curl.h"
#include "Wt-Commons/wt_helpers.h"
#include "Wt-Commons/wform.h"
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
#include "widgets/cataloguesdescriptionwidget.h"
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
#include "exportastrosessionresource.h"
#include <Wt/WSlider>
#include <Wt/WLocalDateTime>
#include "skyplanner.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string/join.hpp>
#include <Wt/Http/Client>
#include <Wt/Http/Message>
#include <Wt/Json/Parser>
#include <Wt/Json/Value>
#include <Wt/Json/Array>
#include <Wt/Json/Object>
#include <Wt/WPopupMenu>
#include <Wt/WMemoryResource>
#include "widgets/filterbytypewidget.h"
#include "widgets/filterbycatalogue.h"
#include "widgets/filterbyconstellation.h"
#include <boost/thread.hpp>
#include <Wt/WStackedWidget>
#include "widgets/astroobjectwidget.h"
#include "widgets/astroobjectstable.h"
#include "geocoder.h"
#include "dbohelper.h"
#include "utils/utils.h"
#include "widgets/positiondetailswidget.h"
#include "widgets/weatherwidget.h"
#include "widgets/texteditordialog.h"
#include "astrosessionpreview.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroSessionTab::Private::Private(const AstroSessionPtr& astroSession, Session &session, AstroSessionTab* q)
  : astroSession(astroSession), session(session), q(q)
{
}

AstroSessionTab::~AstroSessionTab()
{
}

AstroSessionTab::AstroSessionTab(const AstroSessionPtr& astroSession, Session& session, WContainerWidget* parent)
    : WContainerWidget(parent), d(astroSession, session, this)
{
  spLog("notice") << "astroSession: " << astroSession.id() << ", id=" << id();
  d->reload();
}

Signal< NoClass > &AstroSessionTab::close() const
{
  return d->close;
}

template<typename ObjectWidget>
AstroSessionObjectPtr AstroSessionTab::add(const NgcObjectPtr &ngcObject, const AstroSessionPtr &astroSession, Session &session, ObjectWidget *objectWidget)
{
  Dbo::Transaction t(session);
  int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(ngcObject.id() );
  if(existing>0) {
    SkyPlanner::instance()->notification(WString::tr("notification_warning_title"), WString::tr("notification_object_already_added"), SkyPlanner::Notification::Alert, 10);
    return {};
  }
  astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
  auto astroSessionObject = session.find<AstroSessionObject>().where("astro_session_id = ?").bind(astroSession.id()).where("objects_id = ?").bind(ngcObject.id()).resultValue();
  t.commit();
  if(objectWidget)
    objectWidget->addStyleClass("success");

  return astroSessionObject;
}

void AstroSessionTab::add(const Dbo::collection<NgcObjectPtr> &ngcObjects, const AstroSessionPtr &astroSession, Session &session)
{
  Dbo::Transaction t(session);
  for(auto ngcObject: ngcObjects) {
      int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(ngcObject.id() );
      if(existing>0)
	continue;
      astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
  }
}

template AstroSessionObjectPtr AstroSessionTab::add(const NgcObjectPtr &ngcObject, const AstroSessionPtr &astroSession, Session &session, WTableRow *objectWidget);
template AstroSessionObjectPtr AstroSessionTab::add(const NgcObjectPtr &ngcObject, const AstroSessionPtr &astroSession, Session &session, WWidget *objectWidget);
template AstroSessionObjectPtr AstroSessionTab::add(const NgcObjectPtr &ngcObject, const AstroSessionPtr &astroSession, Session &session, WMenuItem *objectWidget);
void AstroSessionTab::Private::reload()
{
  q->clear();
  sessionStacked = new WStackedWidget;
  sessionContainer = new WContainerWidget;
  sessionPreviewContainer = new WContainerWidget;
  sessionStacked->addWidget(sessionContainer);
  sessionStacked->addWidget(sessionPreviewContainer);

  q->addWidget(sessionStacked);
  if(!session.user()) {
    wApp->setInternalPath("/login");
    return;
  }

  Dbo::Transaction t(session);

  auto changeNameOrDateButton = WW<WPushButton>(WString::tr("astrosessiontab_change_name_or_date")).css("btn btn-xs").onClick([=](WMouseEvent){
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
  });

  auto previewVersionButton = WW<WPushButton>(WString::tr("astrosessiontab_preview_version")).css("btn-primary btn-xs").onClick([=](WMouseEvent){
    sessionPreviewContainer->clear();
    auto preview = new AstroSessionPreview{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session, {{"astroobject_remove_from_session", "btn-danger", [=](const AstroSessionObjectPtr &o, AstroObjectWidget* w){
      remove(o, [=] { delete w; });
    } }}};
    sessionPreviewContainer->addWidget(preview);
    sessionStacked->setCurrentWidget(sessionPreviewContainer);
    preview->backClicked().connect([=](_n6){
      sessionPreviewContainer->clear();
      sessionStacked->setCurrentWidget(sessionContainer);
    });
  });
  auto reportButton = WW<WPushButton>(WString::tr("Report")).css("btn-primary btn-xs").onClick([=](WMouseEvent){
    sessionPreviewContainer->clear();
    auto preview = new AstroSessionPreview{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session, {}, AstroSessionPreview::Report};
    sessionPreviewContainer->addWidget(preview);
    sessionStacked->setCurrentWidget(sessionPreviewContainer);
    preview->backClicked().connect([=](_n6){
      sessionPreviewContainer->clear();
      sessionStacked->setCurrentWidget(sessionContainer);
    });
  });

  auto printableVersionButton = WW<WPushButton>(WString::tr("astrosessiontab_printable_version")).css("btn btn-info btn-xs").onClick( [=](WMouseEvent){ printableVersion(); } );

  WPushButton *exportButton = WW<WPushButton>(WString::tr("astrosessiontab_export")).css("btn btn-xs btn-info");
  WPopupMenu *exportMenu = new WPopupMenu;
  exportButton->setMenu(exportMenu);
  for(auto exportType: map<string, ExportAstroSessionResource::ReportType>{
    {"CSV", ExportAstroSessionResource::CSV},
    {"Cartes du Ciel", ExportAstroSessionResource::CartesDuCiel},
#ifndef PRODUCTION_MODE
    {"KStars", ExportAstroSessionResource::KStars},
#endif
  }) {
    WMenuItem *exportMenuItem = exportMenu->addItem(exportType.first);
    delete exportResources[exportType.second];
    exportResources[exportType.second] = new ExportAstroSessionResource(astroSession, session, timezone, exportMenuItem);
    exportResources[exportType.second]->setPlace(geoCoderPlace);
    exportResources[exportType.second]->setReportType(exportType.second);
    exportMenuItem->setLink(exportResources[exportType.second]);
    exportMenuItem->setLinkTarget(TargetNewWindow);
    if(exportType.second == ExportAstroSessionResource::KStars) {
      exportMenuItem->triggered().connect([=](WMenuItem*, _n5) {
	SkyPlanner::instance()->notification(WString::tr("notification_suggestion_title"), WString::tr("kstars_suggestion"), SkyPlanner::Notification::Information);
      });
    }
  }

  auto closeButton = WW<WPushButton>(WString::tr("buttons_close")).css("btn btn-warning btn-xs").onClick( [=](WMouseEvent){ close.emit(); } );

  WForm *actionsContainer = WW<WForm>(WForm::Inline).addCss("hidden-print").setMargin(10);
  WToolBar *actionsToolbar = WW<WToolBar>().addCss("hidden-print");
  sessionContainer->addWidget(actionsContainer);
  actionsToolbar->addButton(changeNameOrDateButton);
  actionsToolbar->addButton(previewVersionButton);
  if(astroSession->wDateWhen() < WDateTime::currentDateTime())
    actionsToolbar->addButton(reportButton);
  actionsToolbar->addButton(printableVersionButton);
  actionsToolbar->addButton(exportButton);
  actionsToolbar->addButton(closeButton);

  actionsContainer->add(actionsToolbar, string{}, false);
  
  WContainerWidget *sessionInfo = WW<WContainerWidget>();
  sessionInfo->addWidget(new WText{WLocalDateTime(astroSession->wDateWhen().date(), astroSession->wDateWhen().time())
    .toString("dddd dd MMMM yyyy")});


  sessionInfo->addWidget(positionDetails = WW<WContainerWidget>());

  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);

  auto locationPanel = WW<WPanel>(addPanel(WString::tr("position_title"), placeWidget, false, true, sessionContainer )).addCss("hidden-print").get();
  addPanel(WString::tr("astrosessiontab_information_panel"), sessionInfo, true, true, sessionContainer)->addStyleClass("hidden-print");
  shared_ptr<SkyPlanner::Notification> placeWidgetInstructions;
  if(astroSession->position()) {
    placeWidget->mapReady().connect([=](_n6){ WTimer::singleShot(1500, [=](WMouseEvent){
        locationPanel->collapse();
      });
    });
  } else {
    placeWidgetInstructions = SkyPlanner::instance()->notification(WString::tr("notification_suggestion_title"), WString::tr("placewidget_instructions_notification"), SkyPlanner::Notification::Information);
  }
  updateTimezone();

  AstroObjectsTable *planetsTable = new AstroObjectsTable(session, {}, AstroObjectsTable::NoFiltersButton, {}, AstroObjectsTable::PlanetColumns());


  planetsTable->planets(astroSession, timezone);

  SelectObjectsWidget *addObjectsTabWidget = new SelectObjectsWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    Dbo::Transaction t(session);
    updateTimezone();
    if(placeWidgetInstructions)
      placeWidgetInstructions->close();
    SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("placewidget_place_set_notification"), SkyPlanner::Notification::Success, 5);
    populate();
    addObjectsTabWidget->populateFor(selectedTelescope, timezone);
    positionDetails->clear();
    positionDetails->addWidget(new PositionDetailsWidget{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session});
    planetsTable->planets(astroSession, timezone);
    if(weatherWidget)
        weatherWidget->reload(astroSession->position(), geoCoderPlace, astroSession->when());
  });
  addPanel(WString::tr("astrosessiontab_add_observable_object"), addObjectsTabWidget, true, true, sessionContainer)->addStyleClass("hidden-print");
  addPanel(WString::tr("astrosessiontab_planets_panel"), planetsTable, true, true, sessionContainer)->addStyleClass("hidden-print");
  if(session.user()->telescopes().size() && session.user()->eyepieces().size()) {
    WTable *instrumentsTable = WW<WTable>().addCss("table table-condensed table-hover table-bordered");
    instrumentsTable->elementAt(0, 0)->addWidget(WW<WText>(WString::tr("telescope")));
    instrumentsTable->elementAt(0, 1)->addWidget(WW<WText>(WString::tr("telescope_dia_focal")));
    instrumentsTable->elementAt(0, 2)->addWidget(WW<WText>(WString::tr("focalMultiplier")));
    instrumentsTable->elementAt(0, 3)->addWidget(WW<WText>(WString::tr("eyepiece")));
    instrumentsTable->elementAt(0, 4)->addWidget(WW<WText>(WString::tr("eyepiece_focal_length")));
    instrumentsTable->elementAt(0, 5)->addWidget(WW<WText>(WString::tr("afov")));
    instrumentsTable->elementAt(0, 6)->addWidget(WW<WText>(WString::tr("magnification")));
    instrumentsTable->elementAt(0, 7)->addWidget(WW<WText>(WString::tr("fov")));
    
    WContainerWidget *instrumentsPanel = WW<WContainerWidget>().addCss("container");
    for(auto telescope: session.user()->telescopes()) {
      for(auto focalMultiplier: session.user()->focalModifiers("no multiplier")) {
	WString focalMultiplierLabel = focalMultiplier->ratio() == 1. ?
	  WString(focalMultiplier->name()) :
	  WString("{1} ({2}x)").arg(focalMultiplier->name()).arg((::format("%.2f") % focalMultiplier->ratio()).str() );
	for(auto eyepiece: session.user()->eyepieces()) {
	  WTableRow *row = instrumentsTable->insertRow(instrumentsTable->rowCount());
	  OpticalSetup setup(telescope, eyepiece, focalMultiplier);
	  row->elementAt(0)->addWidget(WW<WText>(WString::fromUTF8(telescope->name())));
	  row->elementAt(1)->addWidget(WW<WText>(format("%dmm/%dmm") % telescope->diameter() % telescope->focalLength() ));
	  row->elementAt(2)->addWidget(WW<WText>(focalMultiplierLabel));
	  row->elementAt(3)->addWidget(WW<WText>(WString::fromUTF8(eyepiece->name())));
	  row->elementAt(4)->addWidget(WW<WText>(format("%dmm") % eyepiece->focalLength() ));
	  row->elementAt(5)->addWidget(WW<WText>(WString::fromUTF8(eyepiece->aFOV().printable(Angle::IntDegrees))));
	  row->elementAt(6)->addWidget(WW<WText>(format("%.2f") % setup.magnification()));
	  row->elementAt(7)->addWidget(WW<WText>(WString::fromUTF8(setup.fov().printable())));
	}
      }
    }
    addPanel(WString::tr("astrosessiontab_instruments_panel"), instrumentsTable, true, true, sessionContainer);
  }
  auto isPastSession = [=](int slackHours = 0) {
    return (astroSession->when() + boost::posix_time::hours(slackHours)) < boost::posix_time::second_clock().local_time();
  };
  if(!isPastSession(72)) {
      weatherWidget = new WeatherWidget(astroSession->position(), geoCoderPlace, astroSession->when(), WeatherWidget::Full);
      addPanel(WString::tr("weather-panel"), weatherWidget, true, true, sessionContainer)->addStyleClass("hidden-print");
  } else
    weatherWidget = nullptr;
  addObjectsTabWidget->objectsListChanged().connect( [=](const AstroSessionObjectPtr &o, _n5) { populate(o); } );
  WTemplate *title = new WTemplate("<h3 style='display: block'>${tr:astrosessiontab_objects_title} ${counter} \
    <div class='pull-right btn-group' style='display: inline-block'>${expand-button}${filters-button}</div></h3>", sessionContainer);
  title->addFunction("tr", &WTemplate::Functions::tr);
  title->bindWidget("counter", objectsCounter = WW<WText>("0").css("badge"));


  if(timezone)
    sessionContainer->addWidget(  new WText(WString::tr("printable_timezone_info").arg(WString::fromUTF8(timezone.timeZoneName))));

  vector<AstroObjectsTable::Action> actions = {
    {"buttons_extended_info", [](const AstroObjectsTable::Row &r, WWidget*) { r.toggleMoreInfo(); } },
    {"description", [=](const AstroObjectsTable::Row &r, WWidget*) {
      Dbo::Transaction t(session);
      auto sessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(r.astroObject.object.id()).where("astro_session_id = ?").bind(r.astroObject.astroSession.id()).resultValue();
      TextEditorDialog::description( session, sessionObject)->show();
    } },
    {"buttons_remove", [=](const AstroObjectsTable::Row &r, WWidget*) {
      Dbo::Transaction t(session);
      auto sessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(r.astroObject.object.id()).where("astro_session_id = ?").bind(r.astroObject.astroSession.id()).resultValue();
      remove(sessionObject, [=] { populate(); });
    } },
  };
  if(isPastSession(12)) {
    auto toggleObservedStyle = [=](const AstroObjectsTable::Row &r, bool observed) {
      WPushButton *b = reinterpret_cast<WPushButton*>(r.actions.at("astrosessiontab_object_observed_menu"));
      b->setText(observed ? WString::tr("astrosessiontab_object_observed") : WString::tr("astrosessiontab_object_not_observed"));
      b->toggleStyleClass("btn-success", observed);
      r.actions.at("buttons_remove")->setHidden(observed);
      r.actions.at("report")->setHidden(!observed);
    };
    AstroObjectsTable::Action objectReport = AstroObjectsTable::Action{"report", [=](const AstroObjectsTable::Row &r, WWidget *w) {
      Dbo::Transaction t(session);
      auto o = session.find<AstroSessionObject>().where("objects_id = ?").bind(r.astroObject.object.id()).where("astro_session_id = ?").bind(r.astroObject.astroSession.id()).resultValue();
      TextEditorDialog::report(session, o)->show();
    }};
    AstroObjectsTable::Action toggleObserved = AstroObjectsTable::Action{"astrosessiontab_object_observed_menu", [=](const AstroObjectsTable::Row &r, WWidget *w) {
      Dbo::Transaction t(session);
      auto o = session.find<AstroSessionObject>().where("objects_id = ?").bind(r.astroObject.object.id()).where("astro_session_id = ?").bind(r.astroObject.astroSession.id()).resultValue();
      o.modify()->setObserved(!o->observed());
      t.commit();
      WPushButton *b = reinterpret_cast<WPushButton*>(w);
      toggleObservedStyle(r, o->observed());
    }};
    toggleObserved.onButtonCreated = [=](WPushButton *b, const AstroObjectsTable::Row &row) {
      Dbo::Transaction t(session);
      auto o = session.find<AstroSessionObject>().where("objects_id = ?").bind(row.astroObject.object.id()).where("astro_session_id = ?").bind(row.astroObject.astroSession.id()).resultValue();
      toggleObservedStyle(row, o->observed());
    };
    actions.push_back(objectReport);
    actions.push_back(toggleObserved);
  }
  auto columns = AstroObjectsTable::allColumns;
  if(isPastSession(12))
    columns.remove_if([](AstroObjectsTable::Column c){ return c == AstroObjectsTable::Difficulty || c == AstroObjectsTable::ObservationTime || c == AstroObjectsTable::MaxAltitude; });

  sessionContainer->addWidget(astroObjectsTable = new AstroObjectsTable(session, actions, AstroObjectsTable::FiltersButtonExternal, NgcObject::allNebulaTypes(), columns ));
  title->bindWidget("filters-button", WW<WPushButton>(astroObjectsTable->filtersButton()).addCss("btn-sm btn-primary"));
  title->bindWidget("expand-button", WW<WPushButton>(WString::tr("btn-expand-all")).css("btn-sm").onClick([=](WMouseEvent){
    for(auto row: astroObjectsTable->rows())
      row.toggleMoreInfo();
  }));
  astroObjectsTable->forceActionsAsToolBar(isPastSession(12)); 
  astroObjectsTable->objectsListChanged().connect([=](const AstroSessionObjectPtr &o, _n5) { populate(o); });
  astroObjectsTable->filtersChanged().connect([=](AstroObjectsTable::Filters, _n5){ populate(); });
  WContainerWidget *telescopeComboContainer;
  WComboBox *telescopeCombo = WW<WComboBox>().css("input-sm");
  WStandardItemModel *telescopesModel = new WStandardItemModel(sessionContainer);
  telescopeCombo->setModel(telescopesModel);
  WLabel *telescopeComboLabel = WW<WLabel>(WString::tr("astrosessiontab__telescope_label")).setMargin(10);
  telescopeComboLabel->setBuddy(telescopeCombo);
  telescopeComboContainer = actionsContainer->addControl(telescopeCombo, "astrosessiontab__telescope_label");
  
  //actionsContainer->addWidget( telescopeComboContainer = WW<WContainerWidget>().css("form-inline pull-right").add(telescopeComboLabel).add(telescopeCombo));
  telescopeCombo->activated().connect([=](int index, _n5){
    selectedTelescope = boost::any_cast<Dbo::ptr<Telescope>>(telescopesModel->item(index)->data());
    astroObjectsTable->setMagnitudeRange({-5, selectedTelescope->limitMagnitudeGain() + 6.5});
    populate();
    addObjectsTabWidget->populateFor(selectedTelescope, timezone);
  });

  auto updateTelescopes = [=](Dbo::Transaction &t) {
    auto telescopes = session.user()->telescopes();
    if(telescopes.size() > 0) {
      telescopesModel->clear();
      selectedTelescope = {};
      telescopeComboContainer->setHidden(false);
      WStandardItem *defaultItem = 0;
      for(auto telescope: telescopes) {
        WStandardItem *item = new WStandardItem(telescope->name());
        if(!selectedTelescope || telescope->isDefault()) {
          selectedTelescope = telescope;
          spLog("notice") << "Setting telescope " << telescope->name() << " as preselected";
          defaultItem = item;
        }
        item->setData(telescope);
        telescopesModel->appendRow(item);
      }

      if(defaultItem)
        telescopeCombo->setCurrentIndex(telescopesModel->indexFromItem(defaultItem).row());
      astroObjectsTable->setMagnitudeRange({-5, selectedTelescope->limitMagnitudeGain() + 6.5});
    
    } else {
      telescopeComboContainer->setHidden(true);
      WAnchor *telescopeLink =  WW<WAnchor>("", WString::tr("mainmenu_my_telescopes")).css("link alert-link").onClick([=](WMouseEvent){ SkyPlanner::instance()->clearNotifications(); wApp->setInternalPath("/telescopes", true); });
      SkyPlanner::instance()->notification(WString::tr("notification_suggestion_title"), WW<WTemplate>(WString::tr("astrosessiontab_no_telescopes_message")).bindWidget("my_telescopes_link", telescopeLink), SkyPlanner::Notification::Information);
    }
  };
  updateTelescopes(t);
  SkyPlanner::instance()->telescopesListChanged().connect([=](_n6) {
    Dbo::Transaction t(session);
    updateTelescopes(t);
    populate();
    WTimer::singleShot(500, [=](WMouseEvent) {
      addObjectsTabWidget->populateFor(selectedTelescope, timezone);
    });
  });
  
  {
    Ephemeris ephemeris(astroSession->position(), timezone);
    astroObjectsTable->setTimeRange({ephemeris.sun(astroSession->date()).set.utc, ephemeris.sun(astroSession->date()).rise.utc}, timezone);
  }
  
  populate();
  positionDetails->clear();
  positionDetails->addWidget(new PositionDetailsWidget{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session});
  // TODO: something seems to be wrong here...
  // TODO: wait for ready signal?
  WTimer::singleShot(500, [=](WMouseEvent) {
    addObjectsTabWidget->populateFor(selectedTelescope, timezone);
  });
}


string AstroSessionTab::pathComponent(const AstroSessionPtr &astroSession, Dbo::Transaction &transaction)
{
  string nameForMenu = boost::regex_replace(astroSession->name(), boost::regex{"[^a-zA-Z0-9]+"}, "-");
  return format("/sessions/%x/%s") % astroSession.id() % nameForMenu;
}

Wt::Signal<std::string> &AstroSessionTab::nameChanged() const
{
  return d->nameChanged;
}


void AstroSessionTab::Private::updateTimezone()
{
  auto placeInfo = ::GeoCoder::placeInformation(astroSession->position(), astroSession->when());
  timezone = placeInfo.timezone;
  geoCoderPlace = placeInfo.geocoderPlace;
  for(auto resource: exportResources) {
    resource.second->setTimezone(timezone);
    resource.second->setPlace(geoCoderPlace);
  }
}

void AstroSessionTab::Private::printableVersion()
{
  WDialog *printableDialog = new WDialog(WString::tr("astrosessiontab_printable_version"));
  WPushButton *okButton;
  printableDialog->footer()->addWidget(okButton = WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).css("btn btn-primary").onClick([=](WMouseEvent){ printableDialog->accept(); }));
  printableDialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Cancel")).css("btn btn-danger").onClick([=](WMouseEvent){ printableDialog->reject(); }));
  auto printableResource = new ExportAstroSessionResource(astroSession, session, timezone, q);
  printableResource->setPlace(geoCoderPlace);
#ifdef DISABLE_LIBHARU
#define PDF_INDEX -1
#warning "libharu Disabled, export to PDF will not be available"
  printableResource->setReportType(ExportAstroSessionResource::HTML);
#else
#define PDF_INDEX 0
  printableResource->setReportType(ExportAstroSessionResource::PDF);
#endif
  // TODO printableResource->setNamesLimit(1);
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
#ifndef DISABLE_LIBHARU
  formatCombo->addItem("PDF");
#endif
  formatCombo->addItem("HTML");
  formatCombo->activated().connect([=](int r, _n5){
    printableResource->setReportType(r==PDF_INDEX ? ExportAstroSessionResource::PDF : ExportAstroSessionResource::HTML);
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
  
  WComboBox *maxNamesCombo = new WComboBox;
  maxNamesCombo->addItem(WString::tr("max_names_no_limit"));
  for(int i=1; i<11; i++)
    maxNamesCombo->addItem(WString("{1}").arg(i));
  maxNamesCombo->activated().connect([=](int index, _n5){ printableResource->setNamesLimit(index); });
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(new WLabel{WString::tr("astrosessiontab_printable_version_max_names")}).add(maxNamesCombo).add(new WBreak));
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
      printableResource->setTelescope(selectedTelescope);
      printableDialog->contents()->addWidget(new WLabel{WString::tr("printable_version_dialog_telescope_combo_label")});
      WComboBox *telescopesCombo = WW<WComboBox>(printableDialog->contents()).addCss("input-sm");
      WStandardItemModel *telescopesModel = new WStandardItemModel(printableDialog);
      telescopesCombo->setModel(telescopesModel);
      for(auto telescope: telescopes) {
	WStandardItem *item = new WStandardItem(telescope->name());
	item->setData(telescope);
	telescopesModel->appendRow(item);
        if(telescope == selectedTelescope) {
          telescopesCombo->setCurrentIndex(telescopesModel->rowCount()-1);
        }
      }
      telescopesCombo->activated().connect([=](int i, _n5) {
	printableResource->setTelescope(boost::any_cast<Dbo::ptr<Telescope>>(telescopesModel->item(i)->data()));
      });
  }
  printableDialog->show();
}


void AstroSessionTab::Private::remove(const AstroSessionObjectPtr &sessionObject, function<void()> runAfterRemove)
{
      WMessageBox *confirmation = new WMessageBox(WString::tr("messagebox_confirm_removal_title"), WString::tr("messagebox_confirm_removal_message"), Wt::Question, Wt::Ok | Wt::Cancel);
      confirmation->buttonClicked().connect([=](StandardButton b, _n5) {
        if(b != Wt::Ok) {
          confirmation->reject();
          return;
        }
        confirmation->accept();
        Dbo::Transaction t(session);
        astroSession.modify()->astroSessionObjects().erase(sessionObject);
        AstroSessionObjectPtr o = sessionObject;
        o.remove();
        t.commit();
        runAfterRemove();
      });
      confirmation->show();
}

void AstroSessionTab::Private::populate(const AstroSessionObjectPtr &addedObject, int pageNumber)
{
  astroObjectsTable->clear();
  auto filters = astroObjectsTable->currentFilters();
  if(filters.types.size() == 0)
    return;
  Dbo::Transaction t(session);
  Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude}, timezone);
  AstroSessionObject::generateEphemeris(ephemeris, astroSession, timezone, t);
 
  AstroObjectsTable::Page page;
  const string orderByClause{"transit_time ASC, ra asc, dec asc, constellation_abbrev asc"};
  long objectsCount = DboHelper::filterQuery<long>(t, "select count(*) from astro_session_object a inner join objects o on a.objects_id = o.id", filters)
      .where("astro_session_id = ?").bind(astroSession.id()).resultValue();
  if(pageNumber >=0 ) {
    page = AstroObjectsTable::Page::fromCount(pageNumber, objectsCount, [=] (long pageNumber) { populate({}, pageNumber); });
    if(addedObject) {
      typedef Wt::Dbo::dbo_default_traits::IdType ObjectsId;
      vector<ObjectsId> objectsIds; ::Utils::copy(DboHelper::filterQuery<ObjectsId>(t, "SELECT a.id FROM astro_session_object a inner join objects o on a.objects_id = o.id", filters)
                                                  .where("astro_session_id = ?").bind(astroSession.id()).orderBy(orderByClause).resultList(), back_inserter(objectsIds) );
      auto objectRowNumber = std::find(begin(objectsIds), end(objectsIds), addedObject.id() );
      long addedObjectPage = objectRowNumber != objectsIds.end() ? std::distance(begin(objectsIds), objectRowNumber) / page.pageSize : page.current;
      if(addedObjectPage != page.current) {
        WTimer::singleShot(500, [=](WMouseEvent) {
          populate(addedObject, addedObjectPage);
        });
        return;
      }
    }
  }
  auto query = DboHelper::filterQuery<AstroSessionObjectPtr>(t, "select a from astro_session_object a inner join objects o on a.objects_id = o.id", filters, page)
    .where("astro_session_id = ?").bind(astroSession.id())
    .orderBy(orderByClause);
  
  auto sessionObjectsDbCollection = query.resultList();
  
  vector<AstroObjectsTable::AstroObject> astroObjects;
  transform(begin(sessionObjectsDbCollection), end(sessionObjectsDbCollection), back_inserter(astroObjects), [=,&ephemeris, &t](const AstroSessionObjectPtr &o){
    return AstroObjectsTable::AstroObject{o->astroSession(), o->ngcObject(), o->bestAltitude(ephemeris, timezone)};
  });
  
  objectsCounter->setText(format("%d") % objectsCount);

  astroObjectsTable->populate(astroObjects, selectedTelescope, timezone, page,
    addedObject ? AstroObjectsTable::Selection{addedObject->ngcObject(), "success", [=](const AstroObjectsTable::Row &r) {
      SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("notification_object_added").arg(r.tableRow->id()), SkyPlanner::Notification::Information, 5, nullptr, "astrosession_object_added");
    }} : AstroObjectsTable::Selection{} );
  if(page.total > 1) {
    if(page) {
      astroObjectsTable->tableFooter()->addWidget(WW<WPushButton>(WString::tr("astrosessiontab_list_no_pagination")).addCss("btn-link hidden-print").onClick([=](WMouseEvent){ populate({}, -1); }));
      astroObjectsTable->tableFooter()->addWidget(WW<WText>(WString::tr("printable-version-pagination-warning")).addCss("visible-print"));
    }
  }
  if(pageNumber == -1 && astroObjects.size() > page.pageSize)
    astroObjectsTable->tableFooter()->addWidget(WW<WPushButton>(WString::tr("astrosessiontab_list_pagination")).addCss("btn-link hidden-print").onClick([=](WMouseEvent){ populate({}); }));
}


WPanel *AstroSessionTab::Private::addPanel( const WString &title, WWidget *widget, bool collapsed, bool collapsible, WContainerWidget *container )
{
  WPanel *panel = WW<WPanel>(container ? container : q);
  panel->addStyleClass("panel-compact");
  panel->setTitle(title);
  panel->setCollapsible(collapsible);
  panel->setCollapsed(collapsed);
  panel->setCentralWidget(widget);
  return panel;
}

