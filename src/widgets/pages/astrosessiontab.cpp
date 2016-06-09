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
#include "utils/format.h"
#include "utils/curl.h"
#include "wt_helpers.h"
#include "wform.h"
#include "session.h"
#include "widgets/placewidget.h"
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
#include "widgets/skyobjects/selectobjectswidget.h"
#include "widgets/skyobjects/objectdifficultywidget.h"
#include "widgets/skyobjects/objectnameswidget.h"
#include "widgets/skyobjects/cataloguesdescriptionwidget.h"
#include <Wt/Utils>
#include <Wt/WTimer>
#include <boost/format.hpp>
#include <Wt/WImage>
#include <Wt/WLabel>
#include <Wt/WTextArea>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
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
#include "widgets/filters/filterbytypewidget.h"
#include "widgets/filters/filterbycatalogue.h"
#include "widgets/filters/filterbyconstellation.h"
#include <boost/thread.hpp>
#include <Wt/WStackedWidget>
#include "widgets/skyobjects/astroobjectwidget.h"
#include "widgets/skyobjects/astroobjectstable.h"
#include "geocoder.h"
#include "dbohelper.h"
#include "utils/utils.h"
#include "widgets/positiondetailswidget.h"
#include "widgets/weatherwidget.h"
#include "widgets/texteditordialog.h"
#include "widgets/instrumentstable.h"
#include "widgets/moonphasecalendar.h"
#include "widgets/pages/astrosessionpreview.h"
#include "wglyphicon.h"
#include "wt_utils.h"

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
    : WContainerWidget(parent), dptr(astroSession, session, this)
{
  spLog("notice") << "astroSession: " << astroSession.id() << ", id=" << id();
  d->load();
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
    Notifications::instance()->show("notification_warning_title"_wtr, "notification_object_already_added"_wtr, Notification::Alert, 10);
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

void AstroSessionTab::Private::load()
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
  WForm *actionsContainer = WW<WForm>(WForm::Inline).addCss("hidden-print").setMargin(10);
  sessionContainer->addWidget(actionsContainer);

  actionsContainer->add(actionsToolbar(), string{}, false);
  
  sessionInfoWidget = WW<WContainerWidget>();


  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);

  auto locationPanel = WW<WPanel>(addPanel("position_title"_wtr, placeWidget, false, true, sessionContainer )).addCss("hidden-print").get();
  addPanel("astrosessiontab_information_panel"_wtr, sessionInfoWidget, true, true, sessionContainer)->addStyleClass("hidden-print");
  shared_ptr<Notification> placeWidgetInstructions;
  if(astroSession->position()) {
    placeWidget->mapReady().connect([=](_n6){ WTimer::singleShot(1500, [=](WMouseEvent){
        locationPanel->collapse();
      });
    });
  } else {
    placeWidgetInstructions = Notifications::instance()->show("notification_suggestion_title"_wtr, "placewidget_instructions_notification"_wtr, Notification::Information);
  }
  updateTimezone();

  AstroObjectsTable *planetsTable = new AstroObjectsTable(session, {}, AstroObjectsTable::NoFiltersButton, {}, AstroObjectsTable::PlanetColumns());


  planetsTable->planets(astroSession, timezone);

  addObjectsTabWidget = new SelectObjectsWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    Dbo::Transaction t(session);
    updateTimezone();
    if(placeWidgetInstructions)
      placeWidgetInstructions->close();
    Notifications::instance()->show("notification_success_title"_wtr, "placewidget_place_set_notification"_wtr, Notification::Success, 5);
    populate();
    addObjectsTabWidget->populateFor(selectedTelescope, timezone);
    updatePosition();
    planetsTable->planets(astroSession, timezone);
    if(weatherWidget)
        weatherWidget->reload(astroSession->position(), geoCoderPlace, astroSession->when());
  });
  addPanel("astrosessiontab_add_observable_object"_wtr, addObjectsTabWidget, true, true, sessionContainer)->addStyleClass("hidden-print");
  addPanel("astrosessiontab_planets_panel"_wtr, planetsTable, true, true, sessionContainer)->addStyleClass("hidden-print");
  if(session.user()->instruments_ok()) {
    WContainerWidget *instrumentsPanel = WW<WContainerWidget>().addCss("container");
    
    auto instrumentsTable = new InstrumentsTable(session.user(), session);
    addPanel("astrosessiontab_instruments_panel"_wtr, instrumentsTable, true, true, sessionContainer);
  }
  auto isPastSession = [=](int slackHours = 0) {
    return (astroSession->when() + boost::posix_time::hours(slackHours)) < boost::posix_time::second_clock().local_time();
  };
  if(!isPastSession(72)) {
      weatherWidget = new WeatherWidget(astroSession->position(), geoCoderPlace, astroSession->when(), WeatherWidget::Full);
      addPanel("weather-panel"_wtr, weatherWidget, true, true, sessionContainer)->addStyleClass("hidden-print");
  } else
    weatherWidget = nullptr;
  addObjectsTabWidget->objectsListChanged().connect( [=](const AstroSessionObjectPtr &o, _n5) { populate(o); } );
  WTemplate *title = new WTemplate("<h3 style='display: block'>${tr:astrosessiontab_objects_title} ${counter} \
    <div class='pull-right btn-group' style='display: inline-block'>${expand-button}${filters-button}</div></h3>", sessionContainer);
  title->addFunction("tr", &WTemplate::Functions::tr);
  title->bindWidget("counter", objectsCounter = WW<WText>("0").css("badge"));


  if(timezone)
    sessionContainer->addWidget(  new WText("printable_timezone_info"_wtr | WString::fromUTF8(timezone.timeZoneName)));

  vector<AstroObjectsTable::Action> actions = {
    {"buttons_extended_info", [](const auto &row, WWidget*) { row.toggleMoreInfo(); } },
    {"description", [=](const auto &row, WWidget*) {
      Dbo::Transaction t(session);
      auto sessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(row.astroObject.object.id()).where("astro_session_id = ?").bind(row.astroObject.astroSession.id()).resultValue();
      TextEditorDialog::description( session, sessionObject)->show();
    } },
    {"buttons_remove", [=](const auto &row, WWidget*) {
      Dbo::Transaction t(session);
      auto sessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(row.astroObject.object.id()).where("astro_session_id = ?").bind(row.astroObject.astroSession.id()).resultValue();
      this->remove(sessionObject, [=] { this->populate(); });
    } },
  };
  if(isPastSession(12)) {
    auto toggleObservedStyle = [=](const auto &row, bool observed) {
      WPushButton *b = reinterpret_cast<WPushButton*>(row.actions.at("astrosessiontab_object_observed_menu"));
      b->setText(observed ? "astrosessiontab_object_observed"_wtr : "astrosessiontab_object_not_observed"_wtr);
      b->toggleStyleClass("btn-success", observed);
            row.actions.at("buttons_remove")->setHidden(observed);
            row.actions.at("report")->setHidden(!observed);
    };
    AstroObjectsTable::Action objectReport = AstroObjectsTable::Action{"report", [=](const auto &row, WWidget *w) {
      Dbo::Transaction t(session);
      auto o = session.find<AstroSessionObject>().where("objects_id = ?").bind(row.astroObject.object.id()).where("astro_session_id = ?").bind(row.astroObject.astroSession.id()).resultValue();
      TextEditorDialog::report(session, o)->show();
    }};
    AstroObjectsTable::Action toggleObserved = AstroObjectsTable::Action{"astrosessiontab_object_observed_menu", [=](const auto &row, WWidget *w) {
      Dbo::Transaction t(session);
      auto o = session.find<AstroSessionObject>().where("objects_id = ?").bind(row.astroObject.object.id()).where("astro_session_id = ?").bind(row.astroObject.astroSession.id()).resultValue();
      o.modify()->setObserved(!o->observed());
      t.commit();
      WPushButton *b = reinterpret_cast<WPushButton*>(w);
      toggleObservedStyle(row, o->observed());
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
  title->bindWidget("expand-button", WW<WPushButton>("btn-expand-all"_wtr).css("btn-sm").onClick([=](WMouseEvent){
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
  WLabel *telescopeComboLabel = WW<WLabel>("astrosessiontab__telescope_label"_wtr).setMargin(10);
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
      WAnchor *telescopeLink =  WW<WAnchor>("", "mainmenu_my_telescopes"_wtr).css("link alert-link").onClick([=](WMouseEvent){ Notifications::instance()->clear(); wApp->setInternalPath("/telescopes", true); });
      Notifications::instance()->show("notification_suggestion_title"_wtr, WW<WTemplate>("astrosessiontab_no_telescopes_message"_wtr).bindWidget("my_telescopes_link", telescopeLink), Notification::Information);
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
  // TODO: something seems to be wrong here...
  // TODO: wait for ready signal?
//   WTimer::singleShot(500, [=](WMouseEvent) {
//     addObjectsTabWidget->populateFor(selectedTelescope, timezone);
//   });
  updatePosition();
}

void AstroSessionTab::Private::updatePosition()
{
  sessionInfoWidget->clear();
  sessionInfoWidget->addWidget(new WText{WLocalDateTime(astroSession->wDateWhen().date(), astroSession->wDateWhen().time())
    .toString("dddd dd MMMM yyyy")});
  sessionInfoWidget->addWidget(positionDetails = WW<WContainerWidget>());
  positionDetails->addWidget(new PositionDetailsWidget{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session});
  addObjectsTabWidget->populateFor(selectedTelescope, timezone);
}


string AstroSessionTab::pathComponent(const AstroSessionPtr &astroSession, Dbo::Transaction &transaction)
{
  string nameForMenu = boost::regex_replace(astroSession->name(), boost::regex{"[^a-zA-Z0-9]+"}, "-");
  return "/sessions/%x/%s"_s % astroSession.id() % nameForMenu;
}

Wt::Signal<std::string> &AstroSessionTab::nameChanged() const
{
  return d->nameChanged;
}

Wt::Signal<> &AstroSessionTab::sessionsChanged() const
{
  return d->sessionsChanged;
}


WToolBar *AstroSessionTab::Private::actionsToolbar()
{
  auto reportButton = WW<WPushButton>("Report"_wtr).css("btn-primary btn-xs").onClick([=](WMouseEvent){
    sessionPreviewContainer->clear();
    auto preview = new AstroSessionPreview{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session, {}, AstroSessionPreview::Report};
    preview->sessionsChanged().connect([=](_n6){sessionsChanged.emit(); });
    sessionPreviewContainer->addWidget(preview);
    sessionStacked->setCurrentWidget(sessionPreviewContainer);
    preview->backClicked().connect([=](_n6){
      sessionPreviewContainer->clear();
      sessionStacked->setCurrentWidget(sessionContainer);
    });
  }).get();
  
  auto reportButtonVisibility = [=](Dbo::Transaction &t) {
    reportButton->setHidden( astroSession->wDateWhen() >= WDateTime::currentDateTime() );
  };

  auto changeNameOrDateButton = WW<WPushButton>("astrosessiontab_change_name_or_date"_wtr).css("btn btn-xs").onClick([=](WMouseEvent){
    WDialog *changeNameOrDateDialog = new WDialog("astrosessiontab_change_name_or_date"_wtr);
    WLineEdit *sessionName = WW<WLineEdit>(astroSession->name()).css("input-block-level");
    MoonPhaseCalendar::Picker *sessionDate = WW<MoonPhaseCalendar::Picker>(astroSession->wDateWhen().date()).css("input-block-level");
    changeNameOrDateDialog->footer()->addWidget(WW<WPushButton>("Wt.WMessageBox.Ok"_wtr).css("btn btn-primary").onClick([=](WMouseEvent){
      Dbo::Transaction t(session);
      astroSession.modify()->setName(sessionName->text().toUTF8());
      astroSession.modify()->setDateTime(WDateTime{sessionDate->date()});
      session.execute("UPDATE astro_session_object SET azimuth = NULL, altitude = NULL, transit_time = NULL, ephemeris_context_key = NULL WHERE astro_session_id = ?").bind(astroSession.id());
      changeNameOrDateDialog->accept();
      nameChanged.emit(astroSession->name());
      reportButtonVisibility(t);
      updatePosition();
    }));
    WTemplate *form = new WTemplate("<form><fieldset><label>Name</label>${sessionName}<label>Date</label>${sessionDate}</fieldset></form>");
    form->bindWidget("sessionName", sessionName);
    form->bindWidget("sessionDate", sessionDate);
    changeNameOrDateDialog->contents()->addWidget(form);
    changeNameOrDateDialog->show();
  });
  
  auto previewVersionButton = WW<WPushButton>("astrosessiontab_preview_version"_wtr).css("btn-primary btn-xs").onClick([=](WMouseEvent){
    sessionPreviewContainer->clear();
    auto preview = new AstroSessionPreview{{astroSession, selectedTelescope, timezone}, geoCoderPlace, session, {{"astroobject_remove_from_session", "btn-danger", [=](const AstroSessionObjectPtr &o, AstroObjectWidget* w){
      remove(o, [=] { delete w; });
    } }}};
    preview->sessionsChanged().connect([=](_n6){sessionsChanged.emit(); });
    sessionPreviewContainer->addWidget(preview);
    sessionStacked->setCurrentWidget(sessionPreviewContainer);
    preview->backClicked().connect([=](_n6){
      sessionPreviewContainer->clear();
      sessionStacked->setCurrentWidget(sessionContainer);
    });
  });

  auto printableVersionButton = WW<WPushButton>("astrosessiontab_printable_version"_wtr).css("btn btn-info btn-xs").onClick( [=](WMouseEvent){ printableVersion(); } );

  WPushButton *exportButton = WW<WPushButton>("astrosessiontab_export"_wtr).css("btn btn-xs btn-info");
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
        Notifications::instance()->show("notification_suggestion_title"_wtr, "kstars_suggestion"_wtr, Notification::Information);
      });
    }
  }
  auto closeButton = WW<WPushButton>("buttons_close"_wtr).css("btn btn-warning btn-xs").onClick( [=](WMouseEvent){ close.emit(); } );

  WToolBar *actionsToolbar = WW<WToolBar>().addCss("hidden-print");
  actionsToolbar->addButton(changeNameOrDateButton);
  actionsToolbar->addButton(previewVersionButton);
  actionsToolbar->addButton(reportButton);
  actionsToolbar->addButton(printableVersionButton);
  actionsToolbar->addButton(exportButton);
  actionsToolbar->addButton(closeButton);
  Dbo::Transaction t(session);
  reportButtonVisibility(t);
  return actionsToolbar;
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
  WDialog *printableDialog = new WDialog("astrosessiontab_printable_version"_wtr);
  WPushButton *okButton;
  printableDialog->footer()->addWidget(okButton = WW<WPushButton>("Wt.WMessageBox.Ok"_wtr).css("btn btn-primary").onClick([=](WMouseEvent){ printableDialog->accept(); }));
  printableDialog->footer()->addWidget(WW<WPushButton>("Wt.WMessageBox.Cancel"_wtr).css("btn btn-danger").onClick([=](WMouseEvent){ printableDialog->reject(); }));
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
  printableDialog->contents()->addWidget(new WText("printable_version_deprecated_use_preview"_wtr));
  printableDialog->contents()->addWidget(new WBreak);
  printableDialog->contents()->addWidget(new WLabel("printable_version_dialog_spacing_between_objects"_wtr));
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
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(new WLabel{"astrosessiontab_printable_version_dialog_export_as"_wtr}).add(formatCombo).add(new WBreak));
  fontScalingSlider->setWidth(500);
  fontScalingSlider->setMaximum(40);
  fontScalingSlider->setValue(20);
  WText *fontScalingValue = WW<WText>("100%").css("pull-right").setMargin(15, Top);
  fontScalingSlider->valueChanged().connect([=](int v, _n5){
    double value = 2. / 40. * static_cast<double>(v);
    printableResource->setFontScale( value );
    fontScalingValue->setText("%d%%"_ws % static_cast<int>(value*100));
  });
  printableDialog->contents()->addWidget(new WLabel("printable_version_dialog_fonts_size"_wtr));
  printableDialog->contents()->addWidget(new WBreak);
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(fontScalingSlider).add(fontScalingValue));
  printableDialog->contents()->addWidget(new WBreak);
  
  WComboBox *maxNamesCombo = new WComboBox;
  maxNamesCombo->addItem("max_names_no_limit"_wtr);
  for(int i=1; i<11; i++)
    maxNamesCombo->addItem("{1}"_ws  | i);
  maxNamesCombo->activated().connect([=](int index, _n5){ printableResource->setNamesLimit(index); });
  printableDialog->contents()->addWidget(WW<WContainerWidget>().add(new WLabel{"astrosessiontab_printable_version_max_names"_wtr}).add(maxNamesCombo).add(new WBreak));
  Dbo::Transaction t(session);
  auto telescopes = session.user()->telescopes();
  switch(telescopes.size()) {
    case 0:
      printableDialog->contents()->addWidget(new WText{"printable_version_dialog_add_telescope_suggestion"_wtr});
      break;
    case 1:
      printableResource->setTelescope(telescopes.front());
      printableDialog->contents()->addWidget(new WText{"printable_version_dialog_using_telescope"_wtr | telescopes.front()->name() });
      break;
    default:
      printableResource->setTelescope(selectedTelescope);
      printableDialog->contents()->addWidget(new WLabel{"printable_version_dialog_telescope_combo_label"_wtr});
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
      WMessageBox *confirmation = new WMessageBox("messagebox_confirm_removal_title"_wtr, "messagebox_confirm_removal_message"_wtr, Wt::Question, Wt::Ok | Wt::Cancel);
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
  wApp->log("notice") << "pageNumber: " << pageNumber << ", objectsCount: " << objectsCount << ", filters: " << filters;
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
  
  objectsCounter->setText("%d"_ws % objectsCount);

  astroObjectsTable->populate(astroObjects, selectedTelescope, timezone, page,
    addedObject ? AstroObjectsTable::Selection{addedObject->ngcObject(), "success", [=](const AstroObjectsTable::Row &r) {
      Notifications::instance()->show("notification_success_title"_wtr, "notification_object_added"_wtr | r.tableRow->id()
      , Notification::Information, 5, "astrosession_object_added");
    }} : AstroObjectsTable::Selection{} );
  if(page.total > 1) {
    if(page) {
      astroObjectsTable->tableFooter()->addWidget(WW<WPushButton>("astrosessiontab_list_no_pagination"_wtr).addCss("btn-link hidden-print").onClick([=](WMouseEvent){ populate({}, -1); }));
      astroObjectsTable->tableFooter()->addWidget(WW<WText>("printable-version-pagination-warning"_wtr).addCss("visible-print"));
    }
  }
  if(pageNumber == -1 && astroObjects.size() > page.pageSize)
    astroObjectsTable->tableFooter()->addWidget(WW<WPushButton>("astrosessiontab_list_pagination"_wtr).addCss("btn-link hidden-print").onClick([=](WMouseEvent){ populate({}); }));
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

