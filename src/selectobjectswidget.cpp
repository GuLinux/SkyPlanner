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
#include "astroplanner.h"
#include <Wt/WGroupBox>
#include <Wt/WSpinBox>
#include <Wt/Dbo/QueryModel>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

using namespace Wt;
using namespace WtCommons;
using namespace std;


SelectObjectsWidget::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, SelectObjectsWidget* q) : astroSession(astroSession), session(session), q(q)
{
}

SelectObjectsWidget::~SelectObjectsWidget()
{
  d->aborted = true;
  d->bgThread.join();
}

Signal< NoClass >& SelectObjectsWidget::objectsListChanged() const
{
  return d->objectsListChanged;
}


SelectObjectsWidget::SelectObjectsWidget(const Dbo::ptr< AstroSession >& astroSession, Session& session, WContainerWidget* parent)
    : d(astroSession, session, this)
{
    WTabWidget *addObjectsTabWidget = this;
    boost::unique_lock<boost::mutex> lockSession(d->sessionLockMutex);
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
  row->elementAt(0)->addWidget(new ObjectNamesWidget{ngcObject, session, astroSession});
  row->elementAt(1)->addWidget(new WText{ ngcObject->typeDescription() });
  row->elementAt(2)->addWidget(new WText{ WString::fromUTF8(ConstellationFinder::getName(ngcObject->coordinates()).name) });
  row->elementAt(3)->addWidget(new WText{ (ngcObject->magnitude() > 90.) ? "N/A" : (format("%.1f") % ngcObject->magnitude()).str() });
  WDateTime transit = WDateTime::fromPosixTime(bestAltitude.when);
  row->elementAt(4)->addWidget(new ObjectDifficultyWidget(ngcObject, selectedTelescope, 99 /* TODO: hack, to be replaced */));
  row->elementAt(5)->addWidget(new WText{transit.time().toString()});
  row->elementAt(6)->addWidget(new WText{Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable()))});
  row->elementAt(7)->addWidget(WW<WPushButton>(WString::tr("buttons_add")).css("btn btn-primary btn-mini").onClick([=](WMouseEvent){
    Dbo::Transaction t(session);
    int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_id = ? ").bind(astroSession.id() ).bind(ngcObject.id() );
    if(existing>0) {
      AstroPlanner::instance()->notification(WString::tr("notification_warning_title"), WString::tr("notification_object_already_added"), AstroPlanner::Alert, 10);
      return;
    }
    astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
    t.commit();
    row->addStyleClass("success");
    objectsListChanged.emit();
  }));

  // TODO: refactoring
  auto dbDescriptions = ngcObject->descriptions();
  if(!dbDescriptions.empty()) {
    WTableRow *descriptionRow = table->insertRow(table->rowCount());
    WTableCell *descriptionCell = descriptionRow->elementAt(0);
    descriptionCell->setColumnSpan(8);
    descriptionCell->addStyleClass("alert alert-info");
    descriptionCell->addWidget(new WText{WString::tr("object_row_cataloguedesc")});
    for(auto den: dbDescriptions)
      descriptionCell->addWidget(new WText{WString("<strong>{1}</strong>: {2}")
                                           .arg(den.catalogue->name() )
                                           .arg(Utils::htmlEncode( WString::fromUTF8(den.description), Utils::HtmlEncodingFlag::EncodeNewLines )
                                           )
                                 });
  }
}


void SelectObjectsWidget::Private::populateSuggestedObjectsTable()
{
    boost::unique_lock<boost::mutex>(suggestedObjectsListMutex);
    if(suggestedObjectsList.empty() || aborted)
      return;
    auto populateRange = [=] (size_t startOffset, size_t size) {
      Dbo::Transaction transaction(session);
      populateHeaders(suggestedObjectsTable);
      for(size_t i=startOffset; i<min(startOffset+size, suggestedObjectsList.size()); i++) {
        NgcObjectPtr ngcObject = session.find<NgcObject>().where("id = ?").bind(suggestedObjectsList.at(i).first.id());
	Ephemeris::BestAltitude &bestAltitude = suggestedObjectsList.at(i).second;
	append(suggestedObjectsTable, ngcObject, bestAltitude);
      }
    };
    static const int pagesSize = 15;
    suggestedObjectsTablePagination->clear();
    suggestedObjectsTablePagination->setStyleClass("pagination pagination-mini");
    WContainerWidget *paginationWidget = WW<WContainerWidget>();
    paginationWidget->setList(true);
    shared_ptr<vector<WContainerWidget*>> pages(new vector<WContainerWidget*>());
    suggestedObjectsTablePagination->addWidget(paginationWidget);
    
    WContainerWidget *previousButton = WW<WContainerWidget>().css("disabled");
    WContainerWidget *nextButton = WW<WContainerWidget>();
    
    auto activatePage = [=](int pageNumber) {
      if(pageNumber<0 || pageNumber>=pages->size()) return;
      populateRange(pageNumber*pagesSize, pagesSize);
      pages->at(pagesCurrentIndex)->removeStyleClass("active");
      pages->at(pageNumber)->addStyleClass("active");
      pagesCurrentIndex = pageNumber;
      previousButton->setStyleClass(pageNumber == 0 ? "disabled" : "");
      nextButton->setStyleClass(pageNumber == pages->size()-1 ? "disabled" : "");
    };
    
    previousButton->addWidget(WW<WAnchor>("", "&laquo;" ).onClick([=](WMouseEvent){ activatePage(pagesCurrentIndex-1); }));
    nextButton->addWidget(WW<WAnchor>("", "&raquo;" ).onClick([=](WMouseEvent){ activatePage(pagesCurrentIndex+1); }));
    paginationWidget->addWidget(previousButton);
    for(int i=0; i*pagesSize <=suggestedObjectsList.size(); i++) {
      WContainerWidget *page = WW<WContainerWidget>().add(WW<WAnchor>("", boost::lexical_cast<string>(i) ).onClick([=](WMouseEvent){ activatePage(i); }) );
      pages->push_back(page);
      paginationWidget->addWidget(page);
    }
    paginationWidget->addWidget(nextButton);
    pages->at(0)->addStyleClass("active");
    populateRange(0, pagesSize);
    pagesCurrentIndex = 0;
}

#define TelescopeMagnitudeLimit (Wt::UserRole + 1)
void SelectObjectsWidget::Private::suggestedObjects(Dbo::Transaction& transaction)
{
  WContainerWidget *suggestedObjectsContainer = WW<WContainerWidget>();
  WComboBox *astroTypeCombo = new WComboBox;
  WLabel *astroTypeLabel = new WLabel(WString::tr("object_column_type"));
  astroTypeLabel->setBuddy(astroTypeCombo);
  for(int i=NgcObject::AllButStars; i<NgcObject::NebulaTypeCount; i++)
    astroTypeCombo->addItem(NgcObject::typeDescription(static_cast<NgcObject::NebulaType>(i)));
  astroTypeCombo->activated().connect([=](int i, _n5) {
    nebulaTypeFilter = static_cast<NgcObject::NebulaType>(i-2);
    q->populateFor(selectedTelescope);
  });
  
  suggestedObjectsContainer->addWidget(WW<WGroupBox>(WString::tr("filters")).add(WW<WContainerWidget>().css("form-inline").add(astroTypeLabel).add(astroTypeCombo)));
  suggestedObjectsTable = WW<WTable>().addCss("table  table-hover");
  suggestedObjectsTablePagination = WW<WContainerWidget>();
  suggestedObjectsLoaded.connect(this, &SelectObjectsWidget::Private::populateSuggestedObjectsTable);

  suggestedObjectsTable->setHeaderCount(1);
  
  q->addTab(suggestedObjectsContainer, WString::tr("select_objects_widget_best_visible_objects"));
  suggestedObjectsContainer->addWidget(suggestedObjectsTable);
  suggestedObjectsContainer->addWidget(suggestedObjectsTablePagination);
}

void SelectObjectsWidget::Private::populateSuggestedObjectsList( double magnitudeLimit )
{
  boost::unique_lock<boost::mutex> l1(suggestedObjectsListMutex);
  suggestedObjectsTable->clear();
  suggestedObjectsList.clear();
  suggestedObjectsTablePagination->clear();
  WApplication *app = wApp;
  aborted = true;
  bgThread.join();
  aborted = false;
  bgThread = boost::thread([=]{
    boost::unique_lock<boost::mutex> l2(suggestedObjectsListMutex);
    boost::unique_lock<boost::mutex> lockSession(sessionLockMutex);
    Session threadSession;
    Dbo::Transaction t(threadSession);
    auto ngcObjectsQuery = threadSession.find<NgcObject>().where("magnitude < ?").bind(magnitudeLimit);
    if(nebulaTypeFilter != NgcObject::All && nebulaTypeFilter != NgcObject::AllButStars) 
      ngcObjectsQuery.where("\"type\" = ?").bind(nebulaTypeFilter);
    if(nebulaTypeFilter == NgcObject::AllButStars)
      ngcObjectsQuery.where("\"type\" NOT IN (?, ?)").bind(NgcObject::Asterism).bind(NgcObject::RedStar);
 
    dbo::collection<NgcObjectPtr> objects = ngcObjectsQuery.resultList();
    Ephemeris ephemeris(astroSession->position());
    AstroSession::ObservabilityRange range = astroSession->observabilityRange(ephemeris).delta({1,20,0});
    for(auto object: objects) {
      if(aborted) return;
      auto bestAltitude = ephemeris.findBestAltitude(object->coordinates(), range.begin, range.end);
      if(bestAltitude.coordinates.altitude.degrees() > 17.)
        suggestedObjectsList.push_back({object, bestAltitude});
    }

    boost::posix_time::ptime middleRange = range.begin + ((range.end - range.begin) / 2);
    auto observabilityIndex = [&ephemeris,&range,magnitudeLimit,&middleRange] (const NgcObjectPtr &o) {
      double magnitudeDelta = magnitudeLimit - o->magnitude(); // we already know that magnitudeLimit > o->magnitude(), so this is positive
      magnitudeDelta *= 5; // how much?
      double altitude = ephemeris.arDec2altAz(o->coordinates(), middleRange);
      return magnitudeDelta + altitude;
    };
    sort(suggestedObjectsList.rbegin(), suggestedObjectsList.rend(), [&observabilityIndex](const pair<NgcObjectPtr,Ephemeris::BestAltitude> &a, const pair<NgcObjectPtr,Ephemeris::BestAltitude> &b){
      return observabilityIndex(a.first) < observabilityIndex(b.first);
    });
    if(aborted) return;
    WServer::instance()->post(app->sessionId(), [=]{
      suggestedObjectsLoaded.emit();
      app->triggerUpdate();
    });
  });
}


void SelectObjectsWidget::populateFor( const Dbo::ptr< Telescope > &telescope )
{
  double magnitudeLimit = (telescope ? telescope->limitMagnitudeGain() + 6.5 : 12);
  d->selectedTelescope = telescope;
  d->populateSuggestedObjectsList(magnitudeLimit);
}


void SelectObjectsWidget::Private::searchByNameTab(Dbo::Transaction& transaction)
{
  WContainerWidget *addObjectByName = WW<WContainerWidget>();
  WLineEdit *name = WW<WLineEdit>();
  name->setEmptyText(WString::tr("select_objects_widget_add_by_name"));
  WTable *resultsTable = WW<WTable>().addCss("table  table-hover");
  auto searchByName = [=] {
    Dbo::Transaction t(session);
    string nameToSearch = boost::algorithm::trim_copy(name->text().toUTF8());
    boost::replace_all(nameToSearch, "*", "%");
    wApp->log("notice") << "wildcard names search: original=" << name->text() << ", new search pattern: '" << nameToSearch << "'";
    if(lastSearch == nameToSearch)
      return;
    lastSearch = nameToSearch;
    transform(nameToSearch.begin(), nameToSearch.end(), nameToSearch.begin(), ::tolower);
    int count = session.query<int>("select count(*) from denominations where lower(name) like '%' || ? || '%'").bind(nameToSearch);
    wApp->log("notice") << "search by name: count=" << count;
    if(count > 200) { // TODO: pagination
      AstroPlanner::instance()->notification(WString::tr("select_objects_widget_add_by_name"), WString::tr("select_objects_widget_add_by_name_too_many"), AstroPlanner::Information, 5);
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
    .add(WW<WPushButton>(WString::tr("search")).css("btn btn-primary").onClick([=](WMouseEvent){ searchByName(); })));
  addObjectByName->addWidget(resultsTable);
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
  q->addTab(addObjectByCatalogue, WString::tr("select_objects_widget_add_by_catalogue_number"));
}
