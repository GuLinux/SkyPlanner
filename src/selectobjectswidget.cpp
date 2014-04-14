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
#include "utils/utils.h"
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
#include "widgets/astroobjectstable.h"
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
#include "widgets/astroobjectwidget.h"
#include <Wt/WImage>

using namespace Wt;
using namespace WtCommons;
using namespace std;


SelectObjectsWidget::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, SelectObjectsWidget* q) : astroSession(astroSession), session(session),
  addToSessionAction("buttons_add", [=](const AstroObjectsTable::Row &r){ addToSession(r.astroObject.object, r.tableRow);  }, "btn-primary" ),  q(q)
{
}

SelectObjectsWidget::~SelectObjectsWidget()
{
}

Signal< AstroSessionObjectPtr >& SelectObjectsWidget::objectsListChanged() const
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


void SelectObjectsWidget::Private::addToSession(const NgcObjectPtr &ngcObject, WTableRow *row)
{
    Dbo::Transaction t(session);
    int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(ngcObject.id() );
    if(existing>0) {
      SkyPlanner::instance()->notification(WString::tr("notification_warning_title"), WString::tr("notification_object_already_added"), SkyPlanner::Notification::Alert, 10);
      return;
    }
    astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
    auto astroSessionObject = session.find<AstroSessionObject>().where("astro_session_id = ?").bind(astroSession.id()).where("objects_id = ?").bind(ngcObject.id()).resultValue();
    t.commit();
    row->addStyleClass("success");

    objectsListChanged.emit(astroSessionObject);
}


void SelectObjectsWidget::Private::suggestedObjects(Dbo::Transaction& transaction)
{
  WContainerWidget *suggestedObjectsContainer = WW<WContainerWidget>();

  suggestedObjectsTable = new AstroObjectsTable(session, {addToSessionAction}, true, NgcObject::allNebulaTypesButStars());
  suggestedObjectsTable->filtersChanged().connect([=](AstroObjectsTable::Filters, _n5) { populateSuggestedObjectsTable(); });
  suggestedObjectsContainer->setPadding(10);
  q->addTab(suggestedObjectsContainer, WString::tr("select_objects_widget_best_visible_objects"));
  suggestedObjectsContainer->addWidget(suggestedObjectsTable);
}

template<typename T> Dbo::Query<T> SelectObjectsWidget::Private::filterQuery(const std::string &queryString)
{
  CataloguePtr catalogue = suggestedObjectsTable->currentFilters().catalogue;
  Dbo::Query<T> query = session.query<T>( catalogue ? queryString + ", denominations d" : queryString);
  vector<string> filterConditions{suggestedObjectsTable->currentFilters().types.size(), "?"};
  query.where("o.id = ephemeris_cache.objects_id")
    .where("astro_session_id = ?").bind(astroSession.id())
    .where("magnitude >= ?").bind(suggestedObjectsTable->currentFilters().minimumMagnitude);
  if(catalogue) {
    query.where("o.id = d.objects_id")
         .where("d.catalogues_id = ?").bind(catalogue.id());
  }
  query.where(format("\"type\" IN (%s)") % boost::algorithm::join(filterConditions, ", ") );
  for(auto filter: suggestedObjectsTable->currentFilters().types)
    query.bind(filter);
  if(suggestedObjectsTable->currentFilters().constellation)
    query.where("constellation_abbrev = ?").bind(suggestedObjectsTable->currentFilters().constellation.abbrev);
  return query;
}



void SelectObjectsWidget::Private::populateSuggestedObjectsTable( int pageNumber )
{
    suggestedObjectsTable->clear();
    if( suggestedObjectsTable->currentFilters().types.size() == 0) {
      suggestedObjectsTable->tableFooter()->addWidget(WW<WText>(WString::tr("suggested_objects_empty_list")));
      return;
    }
    Dbo::Transaction t(session);
    long objectsCount = filterQuery<long>("select count(*) from objects o, ephemeris_cache").resultValue();
    spLog("notice") << "objects count: " << objectsCount;
    if(objectsCount<=0) {
      suggestedObjectsTable->tableFooter()->addWidget(WW<WText>(WString::tr("suggested_objects_empty_list")));
      return;
    }
  
    auto page = AstroObjectsTable::Page::fromCount(pageNumber, objectsCount, [=](long n) { populateSuggestedObjectsTable(n); } );
    auto ngcObjectsQuery = filterQuery<NgcObjectPtr>("select o from objects o, ephemeris_cache").orderBy("magnitude asc").limit(page.pageSize).offset(page.pageSize * pageNumber);
    suggestedObjectsTable->clear();
    auto results = ngcObjectsQuery.resultList();
    vector<AstroObjectsTable::AstroObject> astroObjects;
    transform(begin(results), end(results), back_inserter(astroObjects), [=,&t](const NgcObjectPtr &o) {
      auto ephemerisCache = session.find<EphemerisCache>().where("astro_session_id = ?").bind(astroSession.id()).where("objects_id = ?").bind(o.id()).limit(1).resultValue();
      return AstroObjectsTable::AstroObject{astroSession, o, ephemerisCache->bestAltitude(timezone), styleFor(o, t)};
    });
    suggestedObjectsTable->populate(astroObjects, selectedTelescope, timezone, page);
}


void SelectObjectsWidget::populateFor(const Dbo::ptr< Telescope > &telescope , Timezone timezone)
{
  d->suggestedObjectsTable->clear();

  double magnitudeLimit = (telescope ? telescope->limitMagnitudeGain() + 6.5 : 12);
  d->suggestedObjectsTable->setMaximumMagnitude(magnitudeLimit - 0.5);
  d->selectedTelescope = telescope;
  d->timezone = timezone;
  if(!d->astroSession->position()) return;
  d->suggestedObjectsTable->tableFooter()->addWidget(WW<WImage>("http://gulinux.net/loading_animation.gif").addCss("center-block"));

  WApplication *app = wApp;
  boost::thread( [=] {
    boost::unique_lock<boost::mutex> lockCachePopulationMutex(d->suggestedObjectsListMutex);
    boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
    WServer::instance()->log("notice") << "Ephemeris cache calculation started.";
    Session ephemerisCacheSession;
    Dbo::Transaction t(ephemerisCacheSession);
    ephemerisCacheSession.execute("delete from ephemeris_cache WHERE astro_session_id = ?").bind(d->astroSession.id());

    Ephemeris ephemeris({d->astroSession->position().latitude, d->astroSession->position().longitude}, d->timezone);
    auto twilight = ephemeris.astronomicalTwilight(d->astroSession->date());
    long loadedObjects = 0;
    for(auto ngcObject: ephemerisCacheSession.find<NgcObject>().where("magnitude < ?").bind(magnitudeLimit).resultList()) {
      auto bestAltitude = ephemeris.findBestAltitude(ngcObject->coordinates(), twilight.set, twilight.rise);
      if(bestAltitude.coordinates.altitude.degrees() > 17.) {
        loadedObjects++;
        ephemerisCacheSession.add(new EphemerisCache{bestAltitude, ngcObject, d->astroSession});
      }
    }
    t.commit();
    WServer::instance()->log("notice") << "Ephemeris cache calculation ended, elapsed: " << boost::posix_time::to_simple_string(boost::posix_time::microsec_clock::local_time() - start) << ", loaded " << loadedObjects << " objects";
    WServer::instance()->post(app->sessionId(), [=] {
      d->populateSuggestedObjectsTable();
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
  AstroObjectsTable *resultsTable = new AstroObjectsTable(session, {addToSessionAction}, false);
  auto searchByNameTrigger = [=] {
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
    
    searchByName(nameToSearch, resultsTable);
  };
  name->changed().connect([=](...){ searchByNameTrigger(); });
  addObjectByName->addWidget(WW<WContainerWidget>().css("form-inline").add(name)
    .add(
         WW<WToolBar>()
            .addButton( WW<WPushButton>(WString::tr("search")).css("btn btn-primary").onClick([=](WMouseEvent){ searchByNameTrigger(); }) )
            .addButton( WW<WPushButton>("?").css("btn btn-primary").onClick([=](WMouseEvent){
                SkyPlanner::instance()->notification(WString::tr("help_notification"), WString::tr("help_search_by_name"), SkyPlanner::Notification::Information, 10 );
            }) )

         ));
  addObjectByName->addWidget(resultsTable);

  addObjectByName->setPadding(10);
  q->addTab(addObjectByName, WString::tr("select_objects_widget_add_by_name"));
}

string SelectObjectsWidget::Private::styleFor( const NgcObjectPtr &object, Dbo::Transaction &t ) const
{
  int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(object.id() );
  return existing > 0 ? "success" : "";
}


void SelectObjectsWidget::Private::searchByName(const string &name, AstroObjectsTable *table, int page)
{
  Dbo::Transaction t(session);
  int count = session.query<int>(R"(select count(distinct o.id) from "objects" o inner join denominations d on o.id = d.objects_id where lower(d.name) like '%' || ? || '%')")
    .bind(name);
  spLog("notice") << "search by name: count=" << count;
  auto tablePage = AstroObjectsTable::Page::fromCount(page, count, [=](int p) { searchByName(name, table, p); });
  if(tablePage.total > 200) {
    SkyPlanner::instance()->notification(WString::tr("select_objects_widget_add_by_name"), WString::tr("select_objects_widget_add_by_name_too_many"), SkyPlanner::Notification::Information, 5);
    return;
  }

  auto ngcObjects = session.query<NgcObjectPtr>(R"(select o from "objects" o inner join denominations d on o.id = d.objects_id where lower(d.name) like '%' || ? || '%' group by o.id)")
    .bind(name).limit(tablePage.pageSize).offset(tablePage.pageSize * page).resultList();
  Ephemeris ephemeris(astroSession->position(), timezone);
  auto twilight = ephemeris.astronomicalTwilight(astroSession->date());
  vector<AstroObjectsTable::AstroObject> objects;
  transform(ngcObjects.begin(), ngcObjects.end(), back_inserter(objects), [=,&ephemeris,&twilight,&t](const NgcObjectPtr &o) {
    auto bestAltitude = ephemeris.findBestAltitude(o->coordinates(), twilight.set, twilight.rise);
    return AstroObjectsTable::AstroObject{astroSession, o, bestAltitude, styleFor(o, t)};
  } );
  table->populate(objects, selectedTelescope, timezone, tablePage);
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
  
  AstroObjectsTable *resultsTable = new AstroObjectsTable(session, {addToSessionAction}, false);

  cataloguesCombo->setModel(cataloguesModel);
  auto searchByCatalogueNumber = [=] {
    Dbo::Transaction t(session);
    string key = string("_bycat: ") + cataloguesCombo->currentText().toUTF8() + catalogueNumber->text().toUTF8();
    if(lastSearch == key)
      return;
    lastSearch = key;
    resultsTable->clear();
    std::string catNumber = boost::algorithm::trim_copy(catalogueNumber->text().toUTF8());
    auto query = session.query<NgcObjectPtr>(R"(select o from "objects" o inner join denominations d on o.id = d.objects_id)")
    .where("d.catalogues_id = ?").bind(cataloguesModel->resultRow(cataloguesCombo->currentIndex()).id());
    if( cataloguesCombo->currentText() == "MCG") {
      catNumber = ::Utils::mcg_name_fix(catNumber);
      query.where("d.number like ?||'%'").bind(catNumber);
    } else {
      query.where("d.number = ? ").bind(catNumber );
    }
    dbo::collection<NgcObjectPtr> objects = query;
    Ephemeris ephemeris(astroSession->position(), timezone);
    auto twilight = ephemeris.astronomicalTwilight(astroSession->date());
    for(auto object: objects) {
      auto bestAltitude = ephemeris.findBestAltitude(object->coordinates(), twilight.set, twilight.rise);
      resultsTable->populate({{astroSession, object, bestAltitude, styleFor(object, t)}}, selectedTelescope, timezone );
    }
  };
  catalogueNumber->changed().connect([=](...){ searchByCatalogueNumber(); });
  addObjectByCatalogue->addWidget(WW<WContainerWidget>().css("form-inline").add(cataloguesCombo).add(catalogueNumber)
    .add(WW<WPushButton>(WString::tr("search")).css("btn btn-primary").onClick([=](WMouseEvent){ searchByCatalogueNumber(); })));
  addObjectByCatalogue->addWidget(resultsTable);
  addObjectByCatalogue->setPadding(10);
  q->addTab(addObjectByCatalogue, WString::tr("select_objects_widget_add_by_catalogue_number"));
}
