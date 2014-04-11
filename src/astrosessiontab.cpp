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
#include "widgets/filterbymagnitudewidget.h"
#include "widgets/filterbytypewidget.h"
#include "widgets/filterbycatalogue.h"
#include "widgets/filterbyconstellation.h"
#include <boost/thread.hpp>
#include <Wt/WStackedWidget>
#include "widgets/astroobjectwidget.h"
#include "widgets/astroobjectstable.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroSessionTab::Private::Private(const Dbo::ptr<AstroSession>& astroSession, Session &session, AstroSessionTab* q)
  : astroSession(astroSession), session(session), client(new Http::Client()), q(q)
{
  client->setTimeout(15);
  client->setMaximumResponseSize(10 * 1024);
  client->done().connect([=](const boost::system::error_code &err, const Http::Message &m, _n4){
    spLog("notice") << "got google response: err=" << err.value() << " [" << err.message() << "]";
    for(auto header: m.headers())
      spLog("notice") << "header: " << header.name() << "=" << header.value();
    spLog("notice") << "status: " << m.status() << ", body: " << m.body();
  });

}

AstroSessionTab::~AstroSessionTab()
{
}

AstroSessionTab::AstroSessionTab(const Dbo::ptr<AstroSession>& astroSession, Session& session, WContainerWidget* parent)
    : WContainerWidget(parent), d(astroSession, session, this)
{
  spLog("notice") << "astroSession: " << astroSession.id() << ", id=" << id();
  d->reload();
}

Signal< NoClass > &AstroSessionTab::close() const
{
  return d->close;
}


void AstroSessionTab::Private::reload()
{
  q->clear();
  WStackedWidget *sessionStacked = new WStackedWidget;
  WContainerWidget *sessionContainer = new WContainerWidget;
  WContainerWidget *sessionPreviewContainer = new WContainerWidget;
  sessionStacked->addWidget(sessionContainer);
  sessionStacked->addWidget(sessionPreviewContainer);

  q->addWidget(sessionStacked);
  if(!session.user()) {
    wApp->setInternalPath("/login");
    return;
  }
  WForm *actionsContainer = WW<WForm>(WForm::Inline).setMargin(10);
  sessionContainer->addWidget(actionsContainer);
  pastObservation = astroSession->wDateWhen() < WDateTime::currentDateTime();


  Dbo::Transaction t(session);

  actionsContainer->addButton(WW<WPushButton>(WString::tr("astrosessiontab_change_name_or_date")).css("btn btn-sm").onClick([=](WMouseEvent){
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
  actionsContainer->addButton(WW<WPushButton>(WString::tr("astrosessiontab_preview_version")).css("btn-primary btn-sm").onClick([=](WMouseEvent){
    spLog("notice") << "Switching to preview version..";
    sessionPreviewContainer->clear();
    sessionPreviewContainer->setStyleClass("astroobjects-list");
    sessionPreviewContainer->addWidget(WW<WText>(WString("<h3>{1}, {2}</h3>").arg(WString::fromUTF8(astroSession->name())).arg(astroSession->wDateWhen().toString("dddd d MMMM yyyy") )).css("text-center") );
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
      populate();
      sessionPreviewContainer->clear();
    });
    WPushButton *invertAllButton = WW<WPushButton>(WString::tr("buttons_invert_all")).css("btn-sm");
    sessionPreviewContainer->addWidget(WW<WToolBar>().addCss("hidden-print pull-right").addButton(backButton).addButton(invertAllButton).addButton(printButton));

    WContainerWidget *infoWidget = WW<WContainerWidget>().css("astroobjects-info-widget");
    updatePositionDetails(infoWidget, false);
    sessionPreviewContainer->addWidget(infoWidget);
    sessionPreviewContainer->addWidget(WW<WText>(WString::tr("dss-embed-menu-info-message")).css("hidden-print"));

    shared_ptr<mutex> downloadImagesMutex(new mutex);
    Dbo::Transaction t(session);

    // TODO: filter this section too?
    auto query = session.query<AstroSessionObjectPtr>("select a from astro_session_object a inner join objects on a.objects_id = objects.id")
        .where("astro_session_id = ?").bind(astroSession.id());

    auto sessionObjectsDbCollection = query.resultList();
    typedef pair<dbo::ptr<AstroSessionObject>, Ephemeris::BestAltitude> AstroSessionObjectElement;
    vector<AstroSessionObjectElement> sessionObjects;
    {
      Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude}, timezone);
      transform(begin(sessionObjectsDbCollection), end(sessionObjectsDbCollection), back_inserter(sessionObjects), [&ephemeris](const dbo::ptr<AstroSessionObject> &o){
        return AstroSessionObjectElement{o, o->bestAltitude(ephemeris)};
      });
    }
    sort(begin(sessionObjects), end(sessionObjects), [&](const AstroSessionObjectElement &a, const AstroSessionObjectElement &b){
      return a.second.when < b.second.when;
    });
    shared_ptr<set<AstroObjectWidget*>> astroObjectWidgets(new set<AstroObjectWidget*>());
    AstroObjectWidget *astroObjectWidget = nullptr;
    for(auto objectelement: sessionObjects) {
      WPushButton *hideButton = WW<WPushButton>(WString::tr("buttons_hide")).css("btn-xs btn-warning");
      WPushButton *collapseButton = WW<WPushButton>(WString::tr("buttons_collapse")).css("btn-xs");
      WPushButton *hideDSSButton = WW<WPushButton>(WString::tr("buttons_hide_dss")).css("btn-xs");
      WPushButton *deleteButton = WW<WPushButton>(WString::tr("astroobject_remove_from_session")).css("btn-xs btn-danger");
      WPushButton *editDescriptionButton = WW<WPushButton>(WString::tr("astroobject_actions_edit_description")).css("btn-xs");
      astroObjectWidget = new AstroObjectWidget(objectelement.first, session, timezone, selectedTelescope, downloadImagesMutex, { editDescriptionButton, collapseButton, hideDSSButton, hideButton, deleteButton });
      astroObjectWidget->addStyleClass("astroobject-list-item");
      hideButton->clicked().connect([=](WMouseEvent){astroObjectWidgets->erase(astroObjectWidget); delete astroObjectWidget; });
      deleteButton->clicked().connect([=](WMouseEvent){ astroObjectWidgets->erase(astroObjectWidget); remove(objectelement.first, [=] { delete astroObjectWidget; }); } );
      hideDSSButton->clicked().connect([=](WMouseEvent){ astroObjectWidget->setDSSVisible(!astroObjectWidget->isDSSVisible()); hideDSSButton->setText(WString::tr( astroObjectWidget->isDSSVisible() ? "buttons_hide_dss" : "buttons_show_dss" ));  });
      collapseButton->clicked().connect([=](WMouseEvent) { astroObjectWidget->setCollapsed(!astroObjectWidget->isCollapsed()); });
      editDescriptionButton->clicked().connect([=](WMouseEvent) {
        Dbo::Transaction t(session);
        WDialog *dialog = new WDialog(WString::tr("astroobject_actions_edit_description"));
        WTextArea *description = WW<WTextArea>(WString::fromUTF8(objectelement.first->description()));
        dialog->contents()->addStyleClass("container");
        dialog->contents()->addWidget(description);
        dialog->setClosable(true);
        dialog->footer()->addWidget(WW<WPushButton>(WString::tr("buttons_save")).onClick([=](WMouseEvent){dialog->accept(); }));
        dialog->finished().connect([=](WDialog::DialogCode r, _n5) {
          if(r != WDialog::Accepted) return;
          Dbo::Transaction t(session);
          objectelement.first.modify()->setDescription(description->text().toUTF8());
          t.commit();
          astroObjectWidget->reload();
        });
        dialog->show();
      });
      astroObjectWidgets->insert(astroObjectWidget);
      sessionPreviewContainer->addWidget(astroObjectWidget);
    }
    if(astroObjectWidget)
      astroObjectWidget->addStyleClass("astroobject-last-list-item");
    invertAllButton->clicked().connect([=](WMouseEvent){ for(auto a: *astroObjectWidgets) a->toggleInvert(); } );
    sessionStacked->setCurrentWidget(sessionPreviewContainer);
  }));
  actionsContainer->addButton(WW<WPushButton>(WString::tr("astrosessiontab_printable_version")).css("btn btn-info btn-sm").onClick( [=](WMouseEvent){ printableVersion(); } ));

  WPushButton *exportButton = WW<WPushButton>(WString::tr("astrosessiontab_export")).css("btn btn-sm btn-info");
  WPopupMenu *exportMenu = new WPopupMenu;
  exportButton->setMenu(exportMenu);
  WMenuItem *exportToCsv = exportMenu->addItem("CSV");
  exportToCsvResource = new ExportAstroSessionResource(astroSession, session, timezone, exportToCsv);
  exportToCsvResource->setReportType(ExportAstroSessionResource::CSV);
  exportToCsv->setLink(exportToCsvResource);
  exportToCsv->setLinkTarget(TargetNewWindow);
  actionsContainer->addButton(exportButton);

  actionsContainer->addButton(WW<WPushButton>(WString::tr("buttons_close")).css("btn btn-warning btn-sm").onClick( [=](WMouseEvent){ close.emit(); } ));

  
  WContainerWidget *sessionInfo = WW<WContainerWidget>();
  sessionInfo->addWidget(new WText{WLocalDateTime(astroSession->wDateWhen().date(), astroSession->wDateWhen().time())
    .toString("dddd dd MMMM yyyy")});


  sessionInfo->addWidget(positionDetails = WW<WContainerWidget>());

  PlaceWidget *placeWidget = new PlaceWidget(astroSession, session);

  auto locationPanel = addPanel(WString::tr("position_title"), placeWidget, false, true, sessionContainer );
  addPanel(WString::tr("astrosessiontab_information_panel"), sessionInfo, true, true, sessionContainer);
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


  SelectObjectsWidget *addObjectsTabWidget = new SelectObjectsWidget(astroSession, session);
  placeWidget->placeChanged().connect([=](double lat, double lng, _n4) {
    updateTimezone();
    if(placeWidgetInstructions)
      placeWidgetInstructions->close();
    SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("placewidget_place_set_notification"), SkyPlanner::Notification::Success, 5);
    addObjectsTabWidget->populateFor(selectedTelescope, timezone);
    updatePositionDetails(positionDetails);
  });
  addPanel(WString::tr("astrosessiontab_add_observable_object"), addObjectsTabWidget, true, true, sessionContainer);
  addObjectsTabWidget->objectsListChanged().connect( [=](const AstroSessionObjectPtr &o, _n5) { populate(o); } );
  WTemplate *title = new WTemplate("<h3>${tr:astrosessiontab_objects_title} ${counter}</h3>", sessionContainer);
  title->addFunction("tr", &WTemplate::Functions::tr);
  title->bindWidget("counter", objectsCounter = WW<WText>("0").css("badge"));

  filterByType = new FilterByTypeWidget(NgcObject::allNebulaTypes());
  filterByMinimumMagnitude = new FilterByMagnitudeWidget({WString::tr("not_set"), {}, WString::tr("minimum_magnitude_label")}, {0, 20});
  filterByType->changed().connect([=](_n6){ populate(); });
  filterByMinimumMagnitude->changed().connect([=](double, _n5){ populate(); });

  filterByCatalogue = new FilterByCatalogue(session);
  filterByCatalogue->changed().connect([=](_n6){ populate(); });
  
  filterByConstellation = new FilterByConstellation;
  filterByConstellation->changed().connect([=](_n6){ populate(); });
  sessionContainer->addWidget(WW<WContainerWidget>().addCss("form-inline").add(filterByType).add(filterByMinimumMagnitude).add(filterByConstellation).add(filterByCatalogue));

  if(timezone)
    sessionContainer->addWidget(  new WText(WString::tr("printable_timezone_info").arg(WString::fromUTF8(timezone.timeZoneName))));

  vector<AstroObjectsTable::Action> actions = {
    {"buttons_extended_info", [](const AstroObjectsTable::Row &r) { r.toggleMoreInfo(); } },
    {"description", [](const AstroObjectsTable::Row &r) { /* TODO */ } },
    {"buttons_remove", [=](const AstroObjectsTable::Row &r) {
      Dbo::Transaction t(session);
      auto sessionObject = session.find<AstroSessionObject>().where("objects_id = ?").bind(r.astroObject.object.id()).where("astro_session_id = ?").bind(r.astroObject.astroSession.id()).resultValue();
      remove(sessionObject, [=] { populate(); });
    } },
  };
  sessionContainer->addWidget(astroObjectsTable = new AstroObjectsTable(session, actions ));
  
  WContainerWidget *telescopeComboContainer;
  WComboBox *telescopeCombo = new WComboBox;
  WStandardItemModel *telescopesModel = new WStandardItemModel(sessionContainer);
  telescopeCombo->setModel(telescopesModel);
  WLabel *telescopeComboLabel = WW<WLabel>(WString::tr("astrosessiontab__telescope_label")).setMargin(10);
  telescopeComboLabel->setBuddy(telescopeCombo);
  telescopeComboContainer = actionsContainer->addControl(telescopeCombo, "astrosessiontab__telescope_label");
  
  //actionsContainer->addWidget( telescopeComboContainer = WW<WContainerWidget>().css("form-inline pull-right").add(telescopeComboLabel).add(telescopeCombo));
  telescopeCombo->activated().connect([=](int index, _n5){
    selectedTelescope = boost::any_cast<Dbo::ptr<Telescope>>(telescopesModel->item(index)->data());
    filterByMinimumMagnitude->setMaximum(selectedTelescope->limitMagnitudeGain() + 6.5);
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
      filterByMinimumMagnitude->setMaximum(selectedTelescope->limitMagnitudeGain() + 6.5);
    
    } else {
      telescopeComboContainer->setHidden(true);
      SkyPlanner::instance()->notification(WString::tr("notification_suggestion_title"), WString::tr("astrosessiontab_no_telescopes_message"), SkyPlanner::Notification::Information, 12);
    }
  };
  updateTelescopes(t);
  SkyPlanner::instance()->telescopesListChanged().connect([=](_n6) {
    Dbo::Transaction t(session);
    updateTelescopes(t);
    populate();
    WTimer::singleShot(200, [=](WMouseEvent) {
      addObjectsTabWidget->populateFor(selectedTelescope, timezone);
    });
  });
  populate();
  updatePositionDetails(positionDetails);
  WTimer::singleShot(200, [=](WMouseEvent) {
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
  static string googleApiKey;
  if(googleApiKey.empty())
    wApp->readConfigurationProperty("google_api_server_key", googleApiKey);

  timezone = Timezone{};
  static map<string,Timezone> timezonesCache;
  if(astroSession->position()){
    string key = Timezone::key(astroSession->position().latitude.degrees(), astroSession->position().longitude.degrees(), astroSession->when(), wApp->locale().name());
    spLog("notice") << "Timezone identifier: " << key;
    if(timezonesCache.count(key)) {
      timezone = timezonesCache[key];
      spLog("notice") << "Timezone " << timezone << " found in cache, skipping webservice request";
    } else {
      string url = format("https://maps.googleapis.com/maps/api/timezone/json?location=%f,%f&timestamp=%d&sensor=false&key=%s&language=%s")
        % astroSession->position().latitude.degrees()
        % astroSession->position().longitude.degrees()
        % astroSession->wDateWhen().toTime_t()
        % googleApiKey
        % wApp->locale().name();
      ;
      spLog("notice") << "URL: " << url;
      stringstream data;
      Curl curl(data);
      bool getRequest = ! googleApiKey.empty() && curl.get(url).requestOk();

      spLog("notice") << "get request: " << boolalpha << getRequest << ", http code: " << curl.httpResponseCode() << ", out: " << data.str();
      if(getRequest) {
        try {
          timezone = Timezone::from(data.str(), astroSession->position().latitude.degrees(), astroSession->position().longitude.degrees());
          timezonesCache[key] = timezone;
          if(exportToCsvResource)
            exportToCsvResource->setTimezone(timezone);
          spLog("notice") << "got timezone info: " << timezone;
        } catch(std::exception &e) {
          spLog("notice") << "Unable to parse json response into a timezone object: " << e.what();
        }
      }
    }
  }

}

void AstroSessionTab::Private::printableVersion()
{
  WDialog *printableDialog = new WDialog(WString::tr("astrosessiontab_printable_version"));
  WPushButton *okButton;
  printableDialog->footer()->addWidget(okButton = WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).css("btn btn-primary").onClick([=](WMouseEvent){ printableDialog->accept(); }));
  printableDialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Cancel")).css("btn btn-danger").onClick([=](WMouseEvent){ printableDialog->reject(); }));
  auto printableResource = new ExportAstroSessionResource(astroSession, session, timezone, q);
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




void AstroSessionTab::Private::updatePositionDetails( WContainerWidget *positionDetails, bool showMeteo )
{
  Dbo::Transaction t(session);
  astroSession.reread();
  positionDetails->clear();
  auto addMoonPhaseDetails = [=](const Ephemeris::LunarPhase &lunarPhase) {
    positionDetails->addWidget(new WText(WString::tr("astrosessiontab_moon_phase").arg(static_cast<int>(lunarPhase.illuminated_fraction * 100 ))));
    positionDetails->addWidget(new WBreak);
  };
  if(!astroSession->position()) {
    addMoonPhaseDetails(Ephemeris({}, {}).moonPhase(astroSession->date()));
    return;
  }
//   forecast.fetch(astroSession->position().longitude, astroSession->position().latitude);
  const Ephemeris ephemeris(astroSession->position(), timezone);
  Ephemeris::RiseTransitSet sun = ephemeris.sun(astroSession->date());
  Ephemeris::RiseTransitSet astroTwilight = ephemeris.astronomicalTwilight(astroSession->date());
  Ephemeris::RiseTransitSet moon = ephemeris.moon(astroSession->date());
  Ephemeris::LunarPhase lunarPhase = ephemeris.moonPhase(astroSession->date());
  Ephemeris::Darkness darkness = ephemeris.darknessHours(astroSession->date() );
  positionDetails->addWidget(new WText{WString::tr("printable_timezone_info").arg(WString::fromUTF8(timezone.timeZoneName))});
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_sun_info"))
    .arg(sun.rise.str(DateTime::DateShort) )
    .arg(sun.set.str(DateTime::DateShort) )
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_astro_twilight_info"))
    .arg(astroTwilight.rise.str(DateTime::DateShort))
    .arg(astroTwilight.set.str(DateTime::DateShort))
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_moon_info"))
    .arg(moon.rise.str(DateTime::DateShort))
    .arg(moon.set.str(DateTime::DateShort))
  ));
  positionDetails->addWidget(new WBreak);


  addMoonPhaseDetails(lunarPhase);
  if(lunarPhase.illuminated_fraction <= 0.5) {

    positionDetails->addWidget(new WText{
      WString::tr("astrosessiontab_darkness_hours")
        .arg(darkness.begin.str(DateTime::DateShort))
        .arg(darkness.end.str(DateTime::DateShort))
        .arg(boost::posix_time::to_simple_string(darkness.duration))
    });
  }

  auto now = boost::posix_time::second_clock::local_time();
  if(showMeteo && astroSession->when() > now && astroSession->when() - now < boost::posix_time::hours(72)) {
    positionDetails->addWidget(new WBreak);
    WAnchor *_7timerLink = new WAnchor{(format("http://7timer.y234.cn/index.php?product=astro&lon=%f&lat=%f&lang=%s&tzshift=0")
      % astroSession->position().longitude.degrees()
      % astroSession->position().latitude.degrees()
      % wApp->locale().name()).str()
      , new WImage(WLink{format("http://www.7timer.com/v4/bin/astro.php?lon=%f&lat=%f&lang=%s&ac=0&unit=metric&tzshift=0")
      % astroSession->position().longitude.degrees() % astroSession->position().latitude.degrees() % wApp->locale().name()
    } )
    };
    _7timerLink->setTarget(TargetNewWindow);
    positionDetails->addWidget(_7timerLink);
  }
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
        Dbo::ptr<AstroSessionObject> o = sessionObject;
        o.remove();
        t.commit();
        runAfterRemove();
      });
      confirmation->show();
}

void AstroSessionTab::Private::populate(const AstroSessionObjectPtr &addedObject)
{
  astroObjectsTable->clear();
  if(filterByType->selected().size() == 0)
    return;
  Dbo::Transaction t(session);
  Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude}, timezone);
  
  auto query = session.query<AstroSessionObjectPtr>(format("select a from astro_session_object a inner join objects on a.objects_id = objects.id %s")
         % ( filterByCatalogue->selectedCatalogue() ? "inner join denominations on objects.id = denominations.objects_id" : "")

       )
      .where("astro_session_id = ?").bind(astroSession.id())
      .where("objects.magnitude > ?").bind(filterByMinimumMagnitude->isMinimum() ? -200 : filterByMinimumMagnitude->magnitude());

  if(filterByCatalogue->selectedCatalogue() )
    query.where("denominations.catalogues_id = ?").bind(filterByCatalogue->selectedCatalogue().id());
  vector<string> filterByTypeConditionPlaceholders{filterByType->selected().size(), "?"};
  query.where(format("\"type\" IN (%s)") % boost::algorithm::join(filterByTypeConditionPlaceholders, ", "));
  for(auto filter: filterByType->selected())
    query.bind(filter);

  if(filterByConstellation->selectedConstellation())
    query.where("objects.constellation_abbrev = ?").bind(filterByConstellation->selectedConstellation().abbrev);

  auto sessionObjectsDbCollection = query.resultList();
  vector<AstroObjectsTable::AstroObject> astroObjects;
  transform(begin(sessionObjectsDbCollection), end(sessionObjectsDbCollection), back_inserter(astroObjects), [&ephemeris](const dbo::ptr<AstroSessionObject> &o){
    return AstroObjectsTable::AstroObject{o->astroSession(), o->ngcObject(), o->bestAltitude(ephemeris)};
  });
  sort(begin(astroObjects), end(astroObjects), [&](const AstroObjectsTable::AstroObject &a, const AstroObjectsTable::AstroObject &b){
    return a.bestAltitude.when < b.bestAltitude.when;
  });
  
  objectsCounter->setText(format("%d") % astroObjects.size());

  astroObjectsTable->populate(astroObjects, selectedTelescope, timezone, addedObject ? AstroObjectsTable::Selection{addedObject->ngcObject(), "success", [=](const AstroObjectsTable::Row &r) {
    SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("notification_object_added").arg(r.tableRow->id()), SkyPlanner::Notification::Information, 5);
  }} : AstroObjectsTable::Selection{} ); 
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

