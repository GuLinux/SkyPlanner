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
    unique_lock<mutex> lockSession(d->sessionLockMutex);
    Dbo::Transaction t(session);
    d->searchByCatalogueTab(t);
    d->suggestedObjects(t);
}

void SelectObjectsWidget::Private::populateHeaders(WTable *table)
{
  table->clear();
  table->elementAt(0, 0)->addWidget(new WText{"Object Names"});
  table->elementAt(0, 1)->addWidget(new WText{"Type"});
  table->elementAt(0, 2)->addWidget(new WText{"Constellation"});
  table->elementAt(0, 3)->addWidget(new WText{"Magnitude"});
  table->elementAt(0, 4)->addWidget(new WText{"Difficulty"});
  table->elementAt(0, 5)->addWidget(new WText{"Transit Time"});
  table->elementAt(0, 6)->addWidget(new WText{"Transit Altitude"});
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
  int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_object_id = ? ").bind(astroSession.id() ).bind(ngcObject->objectId() );
  if(existing > 0)
    row->addStyleClass("success");
  row->elementAt(0)->addWidget(new ObjectNamesWidget{ngcObject, session, astroSession});
  row->elementAt(1)->addWidget(new WText{ ngcObject->typeDescription() });
  row->elementAt(2)->addWidget(new WText{ ConstellationFinder::getName(ngcObject->coordinates()).name });
  row->elementAt(3)->addWidget(new WText{format("%.1f") % ngcObject->magnitude()});
  WDateTime transit = WDateTime::fromPosixTime(bestAltitude.when);
  row->elementAt(4)->addWidget(new ObjectDifficultyWidget(ngcObject, selectedTelescope, 99 /* TODO: hack, to be replaced */));
  row->elementAt(5)->addWidget(new WText{transit.time().toString()});
  row->elementAt(6)->addWidget(new WText{Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable()))});
  row->elementAt(7)->addWidget(WW<WPushButton>("Add").css("btn btn-primary btn-mini").onClick([=](WMouseEvent){
    Dbo::Transaction t(session);
    int existing = session.query<int>("select count(*) from astro_session_object where astro_session_id = ? AND objects_object_id = ? ").bind(astroSession.id() ).bind(ngcObject->objectId() );
    if(existing>0) {
      AstroPlanner::instance()->notification("Warning", "Object already added to this session, skipping.", AstroPlanner::Alert, 10);
      return;
    }
    astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
    t.commit();
    row->addStyleClass("success");
    objectsListChanged.emit();
  }));
}


void SelectObjectsWidget::Private::populateSuggestedObjectsTable()
{
    unique_lock<mutex>(suggestedObjectsListMutex);
    if(suggestedObjectsList.empty() || aborted)
      return;
    auto populateRange = [=] (int startOffset, uint64_t size) {
      Dbo::Transaction transaction(session);
      populateHeaders(suggestedObjectsTable);
      for(int i=startOffset; i<min(startOffset+size, suggestedObjectsList.size()); i++) {
	NgcObjectPtr ngcObject = session.find<NgcObject>().where("object_id = ?").bind(suggestedObjectsList.at(i).first.id());
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
//   suggestedObjectsContainer->setMaximumSize(WLength::Auto, 450);
  suggestedObjectsContainer->setOverflow(WContainerWidget::Overflow::OverflowAuto);
  suggestedObjectsTable = WW<WTable>().addCss("table table-striped table-hover");
  suggestedObjectsTablePagination = WW<WContainerWidget>();
  suggestedObjectsLoaded.connect(this, &SelectObjectsWidget::Private::populateSuggestedObjectsTable);

  suggestedObjectsTable->setHeaderCount(1);
  
  q->addTab(suggestedObjectsContainer, "Best Visible Objects");
  suggestedObjectsContainer->addWidget(suggestedObjectsTable);
  suggestedObjectsContainer->addWidget(suggestedObjectsTablePagination);
}

void SelectObjectsWidget::Private::populateSuggestedObjectsList( double magnitudeLimit )
{
  unique_lock<mutex> l1(suggestedObjectsListMutex);
  suggestedObjectsTable->clear();
  suggestedObjectsList.clear();
  WApplication *app = wApp;
  aborted = true;
  bgThread.join();
  aborted = false;
  bgThread = boost::thread([=]{
    unique_lock<mutex> l2(suggestedObjectsListMutex);
    unique_lock<mutex> lockSession(sessionLockMutex);
    Session threadSession;
    Dbo::Transaction t(threadSession);
    dbo::collection<NgcObjectPtr> objects = threadSession.find<NgcObject>().where("magnitude < ?").bind(magnitudeLimit);
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


void SelectObjectsWidget::Private::searchByCatalogueTab(Dbo::Transaction& transaction)
{
  WContainerWidget *addObjectByCatalogue = WW<WContainerWidget>();
  WComboBox *cataloguesCombo = new WComboBox();
  WLineEdit *catalogueNumber = WW<WLineEdit>();
  WTable *resultsTable = WW<WTable>().addCss("table table-striped table-hover");

  for(auto cat: session.query<string>("select distinct catalogue from denominations WHERE catalogue <> ''").resultList())
    cataloguesCombo->addItem(cat);
  catalogueNumber->setEmptyText("Catalogue Number");
  auto searchByCatalogueNumber = [=] {
    Dbo::Transaction t(session);
    resultsTable->clear();
    dbo::collection<dbo::ptr<NebulaDenomination>> denominations = session.find<NebulaDenomination>().where("catalogue = ?").where("number = ?")
      .bind(cataloguesCombo->currentText()).bind(catalogueNumber->text());
    populateHeaders(resultsTable);
    Ephemeris ephemeris(astroSession->position());
    AstroSession::ObservabilityRange range = astroSession->observabilityRange(ephemeris).delta({1,20,0});
    for(auto nebula: denominations) {
      auto bestAltitude = ephemeris.findBestAltitude(nebula->ngcObject()->coordinates(), range.begin, range.end);
      append(resultsTable, nebula->ngcObject(), bestAltitude);
    }
  };
  catalogueNumber->changed().connect([=](_n1){ searchByCatalogueNumber(); });
  addObjectByCatalogue->addWidget(WW<WContainerWidget>().css("form-inline").add(cataloguesCombo).add(catalogueNumber)
    .add(WW<WPushButton>("Search").css("btn btn-primary").onClick([=](WMouseEvent){ searchByCatalogueNumber(); })));
  addObjectByCatalogue->addWidget(resultsTable);
  q->addTab(addObjectByCatalogue, "Add By Catalogue Number");
}
