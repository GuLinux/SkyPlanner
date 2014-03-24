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
#include "selectobjectswidget.h"
#include "private/selectobjectswidget_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/format.h"
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WTableRow>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>
#include <Wt/WTable>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WLabel>
#include <Wt/Utils>
#include <Wt/WTimer>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include "constellationfinder.h"
#include "widgets/objectnameswidget.h"
#include "widgets/objectdifficultywidget.h"
#include "widgets/cataloguesdescriptionwidget.h"
#include "skyplanner.h"
#include <Wt/WGroupBox>
#include <Wt/WSpinBox>
#include <Wt/WToolBar>
#include <Wt/Dbo/QueryModel>
#include <Wt/WPopupMenu>
#include <Wt/WSlider>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <Wt/WCheckBox>
#include "widgets/filterbytypewidget.h"
#include "widgets/filterbymagnitudewidget.h"
#include "widgets/filterbyconstellation.h"
#include <Wt/WImage>

using namespace Wt;
using namespace WtCommons;
using namespace std;


SelectObjectsWidget::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, SelectObjectsWidget* q) : astroSession(astroSession), session(session), q(q)
{
}

SelectObjectsWidget::~SelectObjectsWidget()
{
}

Signal< NoClass >& SelectObjectsWidget::objectsListChanged() const
{
  return d->objectsListChanged;
}


SelectObjectsWidget::SelectObjectsWidget(const Dbo::ptr< AstroSession >& astroSession, Session& session, WContainerWidget* parent)
    : WTabWidget(parent), d(astroSession, session, this)
{
    WTabWidget *addObjectsTabWidget = this;
    Dbo::Transaction t(session);
    d->suggestedObjects(t);
    d->searchByCatalogueTab(t);
    d->searchByNameTab(t);
}

void SelectObjectsWidget::Private::populateHeaders(WTable *table)
{
  table->clear();
  table->elementAt(0, 0)->addWidget(new WText{WString::tr("object_column_names")});
  table->elementAt(0, 1)->addWidget(new WText{WString::tr("object_column_type")});
  table->elementAt(0, 2)->addWidget(new WText{WString::tr("object_column_constellation")});
  table->elementAt(0, 3)->addWidget(new WText{WString::tr("object_column_magnitude")});
  table->elementAt(0, 4)->addWidget(new WText{WString::tr("object_column_difficulty")});
  table->elementAt(0, 5)->addWidget(new WText{WString::tr("object_column_highest_time")});
  table->elementAt(0, 6)->addWidget(new WText{WString::tr("object_column_max_altitude")});
}

void SelectObjectsWidget::Private::append(WTable *table, const Dbo::ptr<NgcObject> &ngcObject, const Ephemeris::BestAltitude &bestAltitude)
{
  WTableRow *row = table->insertRow(table->rowCount());

  stringstream names;
  string separator = "";
  for(auto denomination: ngcObject->nebulae()) {
    names << separator << denomination->name();
    separator = ", ";
  }
  int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(ngcObject.id() );
  if(existing > 0)
    row->addStyleClass("success");
  row->elementAt(0)->addWidget(WW<ObjectNamesWidget>(new ObjectNamesWidget{ngcObject, session, astroSession}).setInline(true).onClick([=](WMouseEvent) {
    if(selectedRow)
      selectedRow->removeStyleClass("info");
    row->addStyleClass("info");
    selectedRow = row;
  }
  ));
  row->elementAt(1)->addWidget(new WText{ ngcObject->typeDescription() });
  row->elementAt(2)->addWidget(new WText{ WString::fromUTF8(ngcObject->constellation().name) });
  row->elementAt(3)->addWidget(new WText{ (ngcObject->magnitude() > 90.) ? "N/A" : (format("%.1f") % ngcObject->magnitude()).str() });
  WDateTime transit = WDateTime::fromPosixTime(timezone.fix(bestAltitude.when));
  row->elementAt(4)->addWidget(new ObjectDifficultyWidget(ngcObject, selectedTelescope, 99 /* TODO: hack, to be replaced */));
  row->elementAt(5)->addWidget(new WText{transit.time().toString()});
  row->elementAt(6)->addWidget(new WText{Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable()))});
  row->elementAt(7)->addWidget(WW<WPushButton>(WString::tr("buttons_add")).css("btn btn-primary btn-xs").onClick([=](WMouseEvent){
    Dbo::Transaction t(session);
    int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(ngcObject.id() );
    if(existing>0) {
      SkyPlanner::instance()->notification(WString::tr("notification_warning_title"), WString::tr("notification_object_already_added"), SkyPlanner::Notification::Alert, 10);
      return;
    }
    astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
    t.commit();
    row->addStyleClass("success");
    objectsListChanged.emit();
  }));

  auto cataloguesDescriptionWidget = CataloguesDescriptionWidget::add(table, 8, ngcObject, true);
  if(cataloguesDescriptionWidget) {
    WPushButton *toggleDescriptionsButton = WW<WPushButton>(WString::tr("object_more_info"), row->elementAt(0)).css("btn btn-xs pull-right");
    toggleDescriptionsButton->clicked().connect([=](WMouseEvent){
      toggleDescriptionsButton->toggleStyleClass("btn-inverse", cataloguesDescriptionWidget->isHidden());
      cataloguesDescriptionWidget->setHidden(!cataloguesDescriptionWidget->isHidden());
    });
  }
}


void SelectObjectsWidget::Private::suggestedObjects(Dbo::Transaction& transaction)
{
  WContainerWidget *suggestedObjectsContainer = WW<WContainerWidget>();

  filterByMinimumMagnitude = new FilterByMagnitudeWidget({WString::tr("not_set"), {}, WString::tr("minimum_magnitude_label")}, {0, 20});
  filterByMinimumMagnitude->changed().connect([=](double, _n5) { populateSuggestedObjectsList(); });
  filterByTypeWidget = new FilterByTypeWidget(NgcObject::allNebulaTypesButStars());
  filterByTypeWidget->changed().connect([=](_n6){ populateSuggestedObjectsList(); });
  filterByConstellation = new FilterByConstellation;
  filterByConstellation->changed().connect([=](_n6){ populateSuggestedObjectsList(); });
  suggestedObjectsContainer->addWidget(WW<WGroupBox>(WString::tr("filters")).add(WW<WContainerWidget>().css("form-inline").add(filterByTypeWidget).add(filterByMinimumMagnitude).add(filterByConstellation) ));
  suggestedObjectsTable = WW<WTable>().addCss("table  table-hover");
  suggestedObjectsTablePagination = WW<WContainerWidget>();

  suggestedObjectsTable->setHeaderCount(1);
  
  suggestedObjectsContainer->setPadding(10);
  q->addTab(suggestedObjectsContainer, WString::tr("select_objects_widget_best_visible_objects"));
  suggestedObjectsContainer->addWidget(suggestedObjectsTable);
  suggestedObjectsContainer->addWidget(suggestedObjectsTablePagination);
}

template<typename T> T &SelectObjectsWidget::Private::filterQuery(T &query)
{
  vector<string> filterConditions{filterByTypeWidget->selected().size(), "?"};
  query.where("o.id = ephemeris_cache.objects_id")
    .where("astro_session_id = ?").bind(astroSession.id())
    .where("magnitude >= ?").bind(filterByMinimumMagnitude->isMinimum() ? -20 : filterByMinimumMagnitude->magnitude());
  query.where(format("\"type\" IN (%s)") % boost::algorithm::join(filterConditions, ", ") );
  for(auto filter: filterByTypeWidget->selected())
    query.bind(filter);
  if(filterByConstellation->selectedConstellation())
    query.where("constellation_abbrev = ?").bind(filterByConstellation->selectedConstellation().abbrev);
  return query;
}

void SelectObjectsWidget::Private::populateSuggestedObjectsList()
{
  suggestedObjectsTable->clear();
  suggestedObjectsTablePagination->clear();
  selectedRow = 0;
  
  if(filterByTypeWidget->selected().size() == 0)
    return;
  Dbo::Transaction t(session);
  double minimumMagnitude = filterByMinimumMagnitude->isMinimum() ? -20 : filterByMinimumMagnitude->magnitude();
  vector<string> filterConditions{filterByTypeWidget->selected().size(), "?"};
  
  auto objectsCountQuery = session.query<long>("select count(*) from objects o, ephemeris_cache");
  long objectsCount = filterQuery(objectsCountQuery).resultValue();
  
  spLog("notice") << "objects count: " << objectsCount;
  
  auto populateTable = [=](long limit, long offset) {
    Dbo::Transaction t(session);
    auto ngcObjectsQuery = session.query<NgcObjectPtr>("select o from objects o, ephemeris_cache");
    filterQuery(ngcObjectsQuery).orderBy("magnitude asc").limit(limit).offset(offset);
    suggestedObjectsTable->clear();
    populateHeaders(suggestedObjectsTable);
    for(auto ngcObject: ngcObjectsQuery.resultList() ) {
      auto ephemerisCache = session.find<EphemerisCache>().where("astro_session_id = ?").bind(astroSession.id()).where("objects_id = ?").bind(ngcObject.id()).resultValue();
      append(suggestedObjectsTable, ngcObject, ephemerisCache->bestAltitude());
    }
  };
  
  int pagesSize = 15;
  
  
  WContainerWidget *paginationWidget = WW<WContainerWidget>().addCss("pagination pagination-sm");
  paginationWidget->setList(true);
  shared_ptr<vector<WContainerWidget*>> pages(new vector<WContainerWidget*>());
  suggestedObjectsTablePagination->addWidget(paginationWidget);
  
  WContainerWidget *previousButton = WW<WContainerWidget>().css("disabled");
  WContainerWidget *nextButton = WW<WContainerWidget>();
  
  auto activatePage = [=](int pageNumber) {
    if(pageNumber<0 || pageNumber>=pages->size()) return;
    populateTable(pagesSize, pageNumber*pagesSize);
    
    pages->at(pagesCurrentIndex)->removeStyleClass("active");
    pages->at(pageNumber)->addStyleClass("active");
    pagesCurrentIndex = pageNumber;
    previousButton->setStyleClass(pageNumber == 0 ? "disabled" : "");
    nextButton->setStyleClass(pageNumber == pages->size()-1 ? "disabled" : "");
  };
  
  previousButton->addWidget(WW<WAnchor>("", "&laquo;" ).onClick([=](WMouseEvent){ activatePage(pagesCurrentIndex-1); }));
  nextButton->addWidget(WW<WAnchor>("", "&raquo;" ).onClick([=](WMouseEvent){ activatePage(pagesCurrentIndex+1); }));
  paginationWidget->addWidget(previousButton);
  for(int i=0; i*pagesSize <objectsCount; i++) {
    WContainerWidget *page = WW<WContainerWidget>().add(WW<WAnchor>("", boost::lexical_cast<string>(i+1) ).onClick([=](WMouseEvent){ activatePage(i); }) );
    pages->push_back(page);
    paginationWidget->addWidget(page);
  }
  paginationWidget->addWidget(nextButton);
  pages->at(0)->addStyleClass("active");
  pagesCurrentIndex = 0;
  populateTable(pagesSize, 0);
}


void SelectObjectsWidget::populateFor(const Dbo::ptr< Telescope > &telescope , Timezone timezone)
{
  d->suggestedObjectsTablePagination->clear();
  d->suggestedObjectsTable->clear();
  d->suggestedObjectsTablePagination->addWidget(WW<WImage>("http://gulinux.net/loading_animation.gif").addCss("center-block"));
  double magnitudeLimit = (telescope ? telescope->limitMagnitudeGain() + 6.5 : 12);
  d->filterByMinimumMagnitude->setMaximum(magnitudeLimit-0.5);
  d->selectedTelescope = telescope;
  d->timezone = timezone;
  if(!d->astroSession->position()) return;
  WApplication *app = wApp;
  boost::thread( [=] {
    boost::unique_lock<boost::mutex> lockCachePopulationMutex(d->suggestedObjectsListMutex);
    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
    WServer::instance()->log("notice") << "Ephemeris cache calculation started.";
    Session ephemerisCacheSession;
    Dbo::Transaction t(ephemerisCacheSession);
    ephemerisCacheSession.execute("delete from ephemeris_cache WHERE astro_session_id = ?").bind(d->astroSession.id());

    Ephemeris ephemeris({d->astroSession->position().latitude, d->astroSession->position().longitude});
    AstroSession::ObservabilityRange range = d->astroSession->observabilityRange(ephemeris).delta({1,20,0});
    for(auto ngcObject: ephemerisCacheSession.find<NgcObject>().where("magnitude < ?").bind(magnitudeLimit).resultList()) {
      auto bestAltitude = ephemeris.findBestAltitude(ngcObject->coordinates(), range.begin, range.end);
      if(bestAltitude.coordinates.altitude.degrees() > 17.) {
        ephemerisCacheSession.add(new EphemerisCache{bestAltitude, ngcObject, d->astroSession});
      }
    }
    WServer::instance()->log("notice") << "Ephemeris cache calculation ended, elapsed: " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time() - start);
    WServer::instance()->post(app->sessionId(), [=] {
      d->populateSuggestedObjectsList();
      app->triggerUpdate();
    });
  }).detach();
}


void SelectObjectsWidget::Private::searchByNameTab(Dbo::Transaction& transaction)
{
  WContainerWidget *addObjectByName = WW<WContainerWidget>();
  WLineEdit *name = WW<WLineEdit>();
  name->setTextSize(0);
  name->setEmptyText(WString::tr("select_objects_widget_add_by_name"));
  WTable *resultsTable = WW<WTable>().addCss("table  table-hover");
  auto searchByName = [=] {
    Dbo::Transaction t(session);
    string nameToSearch = boost::algorithm::trim_copy(name->text().toUTF8());
    boost::replace_all(nameToSearch, "*", "%");
    if(nameToSearch.empty()) {
      resultsTable->clear();
      return;
    }
    
    spLog("notice") << "wildcard names search: original=" << name->text() << ", new search pattern: '" << nameToSearch << "'";
    if(lastSearch == nameToSearch)
      return;
    lastSearch = nameToSearch;
    transform(nameToSearch.begin(), nameToSearch.end(), nameToSearch.begin(), ::tolower);
    int count = session.query<int>("select count(*) from denominations where lower(name) like '%' || ? || '%'").bind(nameToSearch);
    spLog("notice") << "search by name: count=" << count;
    if(count > 200) { // TODO: pagination
      SkyPlanner::instance()->notification(WString::tr("select_objects_widget_add_by_name"), WString::tr("select_objects_widget_add_by_name_too_many"), SkyPlanner::Notification::Information, 5);
      return;
    }
    resultsTable->clear();
    dbo::collection<dbo::ptr<NebulaDenomination>> dboDenominations = session.find<NebulaDenomination>().where("lower(name) like '%' || ? || '%' ")
     .bind(nameToSearch);
    vector<NebulaDenominationPtr> denominations;
    copy_if(begin(dboDenominations), end(dboDenominations), back_inserter(denominations), [&denominations](const NebulaDenominationPtr &a){
      return count_if(begin(denominations), end(denominations), [&a](const NebulaDenominationPtr &b){ return a->ngcObject().id() == b->ngcObject().id(); }) == 0;
    });

    populateHeaders(resultsTable);
    Ephemeris ephemeris(astroSession->position());
    AstroSession::ObservabilityRange range = astroSession->observabilityRange(ephemeris).delta({1,20,0});
    for(auto nebula: denominations) {
      auto bestAltitude = ephemeris.findBestAltitude(nebula->ngcObject()->coordinates(), range.begin, range.end);
      append(resultsTable, nebula->ngcObject(), bestAltitude);
    }
  };
  name->changed().connect([=](...){ searchByName(); });
  addObjectByName->addWidget(WW<WContainerWidget>().css("form-inline").add(name)
    .add(
         WW<WToolBar>()
            .addButton( WW<WPushButton>(WString::tr("search")).css("btn btn-primary").onClick([=](WMouseEvent){ searchByName(); }) )
            .addButton( WW<WPushButton>("?").css("btn btn-primary").onClick([=](WMouseEvent){
                SkyPlanner::instance()->notification(WString::tr("help_notification"), WString::tr("help_search_by_name"), SkyPlanner::Notification::Information, 10 );
            }) )

         ));
  addObjectByName->addWidget(resultsTable);

  addObjectByName->setPadding(10);
  q->addTab(addObjectByName, WString::tr("select_objects_widget_add_by_name"));
}

void SelectObjectsWidget::Private::searchByCatalogueTab(Dbo::Transaction& transaction)
{
  WContainerWidget *addObjectByCatalogue = WW<WContainerWidget>();
  WComboBox *cataloguesCombo = new WComboBox();
  Dbo::QueryModel<CataloguePtr> *cataloguesModel = new Dbo::QueryModel<CataloguePtr>(q);
  cataloguesModel->setQuery(session.find<Catalogue>().where("hidden = ?").bind(false).orderBy("priority asc"));
  cataloguesModel->addColumn("name");
  cataloguesModel->addColumn("id");
  WLineEdit *catalogueNumber = WW<WLineEdit>();
  catalogueNumber->setTextSize(0);
  catalogueNumber->setEmptyText(WString::tr("catalogue_number"));
  WTable *resultsTable = WW<WTable>().addCss("table  table-hover");


  cataloguesCombo->setModel(cataloguesModel);
  auto searchByCatalogueNumber = [=] {
    Dbo::Transaction t(session);
    string key = string("_bycat: ") + cataloguesCombo->currentText().toUTF8() + catalogueNumber->text().toUTF8();
    if(lastSearch == key)
      return;
    lastSearch = key;
    resultsTable->clear();
    dbo::collection<dbo::ptr<NebulaDenomination>> dboDenominations = session.find<NebulaDenomination>().where("catalogues_id = ?").where("number = ? ")
     .bind( cataloguesModel->resultRow(cataloguesCombo->currentIndex()).id() )
     .bind(boost::algorithm::trim_copy(catalogueNumber->text().toUTF8()));
    vector<NebulaDenominationPtr> denominations;
    copy_if(begin(dboDenominations), end(dboDenominations), back_inserter(denominations), [&denominations](const NebulaDenominationPtr &a){
      return count_if(begin(denominations), end(denominations), [&a](const NebulaDenominationPtr &b){ return a->ngcObject().id() == b->ngcObject().id(); }) == 0;
    });
    populateHeaders(resultsTable);
    Ephemeris ephemeris(astroSession->position());
    AstroSession::ObservabilityRange range = astroSession->observabilityRange(ephemeris).delta({1,20,0});
    for(auto nebula: denominations) {
      auto bestAltitude = ephemeris.findBestAltitude(nebula->ngcObject()->coordinates(), range.begin, range.end);
      append(resultsTable, nebula->ngcObject(), bestAltitude);
    }
  };
  catalogueNumber->changed().connect([=](...){ searchByCatalogueNumber(); });
  addObjectByCatalogue->addWidget(WW<WContainerWidget>().css("form-inline").add(cataloguesCombo).add(catalogueNumber)
    .add(WW<WPushButton>(WString::tr("search")).css("btn btn-primary").onClick([=](WMouseEvent){ searchByCatalogueNumber(); })));
  addObjectByCatalogue->addWidget(resultsTable);
  addObjectByCatalogue->setPadding(10);
  q->addTab(addObjectByCatalogue, WString::tr("select_objects_widget_add_by_catalogue_number"));
}
