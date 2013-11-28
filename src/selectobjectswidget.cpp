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
#include <boost/format.hpp>

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
    : d(astroSession, session, this)
{
    WTabWidget *addObjectsTabWidget = this;
    Dbo::Transaction t(session);
    d->searchByCatalogueTab(t);
    d->suggestedObjects(t);
}

#define TelescopeMagnitudeLimit (Wt::UserRole + 1)
void SelectObjectsWidget::Private::suggestedObjects(Dbo::Transaction& transaction)
{
  WContainerWidget *suggestedObjectsContainer = WW<WContainerWidget>();
  suggestedObjectsContainer->setMaximumSize(WLength::Auto, 450);
  suggestedObjectsContainer->setOverflow(WContainerWidget::Overflow::OverflowAuto);
  WTable *resultsTable = WW<WTable>().addCss("table table-striped table-hover");
  resultsTable->setHeaderCount(1);
  auto populateTable = [=](double magnitudeLimit) {
    resultsTable->clear();
    resultsTable->elementAt(0, 0)->addWidget(new WText{"Object Names"});
    resultsTable->elementAt(0, 1)->addWidget(new WText{"Magnitude"});
    resultsTable->elementAt(0, 2)->addWidget(new WText{"Transit Time"});
    resultsTable->elementAt(0, 3)->addWidget(new WText{"Transit Altitude"});
    Dbo::Transaction t(session);
    dbo::collection<NgcObjectPtr> objects = session.find<NgcObject>().where("magnitude < ?").bind(magnitudeLimit);
    vector<NgcObjectPtr> filteredByTransit;
    Ephemeris ephemeris(astroSession->position());
    AstroSession::ObservabilityRange range = astroSession->observabilityRange(ephemeris).delta({1,20,0});
    copy_if(begin(objects), end(objects), back_inserter(filteredByTransit), [&ephemeris,&range](const NgcObjectPtr &o){
      auto bestAltitude = ephemeris.findBestAltitude(o->coordinates(), range.begin, range.end);
      return bestAltitude.coordinates.altitude.degrees() > 20;
    });
    boost::posix_time::ptime middleRange = range.begin + ((range.end - range.begin) / 2);
    auto observabilityIndex = [&ephemeris,&range,magnitudeLimit,&middleRange] (const NgcObjectPtr &o) {
      double magnitudeDelta = magnitudeLimit - o->magnitude(); // we already know that magnitudeLimit > o->magnitude(), so this is positive
      magnitudeDelta *= 20; // how much?
      double altitude = ephemeris.arDec2altAz(o->coordinates(), middleRange);
      return magnitudeDelta + altitude;
    };
    sort(filteredByTransit.rbegin(), filteredByTransit.rend(), [&observabilityIndex](const NgcObjectPtr &a, const NgcObjectPtr &b){
      return observabilityIndex(a) < observabilityIndex(b);
    });
    
    for(NgcObjectPtr ngcObject: filteredByTransit) {
      WTableRow *row = resultsTable->insertRow(resultsTable->rowCount());
      stringstream names;
      string separator = "";
      for(auto denomination: ngcObject->nebulae()) {
        names << separator << denomination->name();
        separator = ", ";
      }
      row->elementAt(0)->addWidget(new WText{Utils::htmlEncode(WString::fromUTF8(names.str()))});
      row->elementAt(1)->addWidget(new WText{(boost::format("%.3f") % ngcObject->magnitude()).str()});
      auto bestAltitude = ephemeris.findBestAltitude(ngcObject->coordinates(), range.begin, range.end);
      WDateTime transit = WDateTime::fromPosixTime(bestAltitude.when);
      row->elementAt(2)->addWidget(new WText{transit.time().toString()});
      row->elementAt(3)->addWidget(new WText{Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable()))});
      row->elementAt(4)->addWidget(WW<WPushButton>("Add").css("btn btn-primary").onClick([=](WMouseEvent){
        Dbo::Transaction t(session);
        astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(ngcObject));
        t.commit();
        objectsListChanged.emit();
      }));
    }
  };

  
  q->addTab(suggestedObjectsContainer, "Best Visible Objects");
  auto telescopes = session.user()->telescopes();
  if(telescopes.size() > 0) {
    WComboBox *telescopesCombo = WW<WComboBox>();
    WLabel *telescopesComboLabel = new WLabel("Telescope: ");
    telescopesComboLabel->setBuddy(telescopesCombo);
    WStandardItemModel *telescopesModel = new WStandardItemModel(telescopesCombo);
    telescopesCombo->setModel(telescopesModel);
    telescopesCombo->activated().connect([=](int which, _n5){
      double magnitude = boost::any_cast<double>(telescopesModel->item(which)->data(TelescopeMagnitudeLimit));
      populateTable(magnitude);
    });
    for(auto telescope: telescopes) {
      WStandardItem *item = new WStandardItem(telescope->name());
      item->setData(telescope->limitMagnitudeGain() + 6.5, TelescopeMagnitudeLimit);
      telescopesModel->appendRow(item);
    }
    suggestedObjectsContainer->addWidget(WW<WContainerWidget>().css("form-inline").add(telescopesComboLabel).add(telescopesCombo));
    suggestedObjectsContainer->addWidget(resultsTable);
    double magnitude = boost::any_cast<double>(telescopesModel->item(telescopesCombo->currentIndex())->data(TelescopeMagnitudeLimit));
    populateTable(magnitude);
  } else {
    suggestedObjectsContainer->addWidget(new WText{"Please add one or more telescope in the \"My Telescopes\" section to have customized suggestions.<br />\
      In the meantime objects up to magnitude 12 will be shown here."});
    suggestedObjectsContainer->addWidget(resultsTable);
    populateTable(12);
  }
}

void SelectObjectsWidget::Private::searchByCatalogueTab(Dbo::Transaction& transaction)
{
  WContainerWidget *addObjectByCatalogue = WW<WContainerWidget>();
  WComboBox *cataloguesCombo = new WComboBox();
  WLineEdit *catalogueNumber = WW<WLineEdit>();
  WTable *resultsTable = WW<WTable>().addCss("table table-striped table-hover");

  // TODO: .......
  for(auto cat: vector<string>{"Messier", "NGC", "IC", "Caldwell"})
    cataloguesCombo->addItem(cat);
  catalogueNumber->setEmptyText("Catalogue Number");
  auto searchByCatalogueNumber = [=] {
    Dbo::Transaction t(session);
    resultsTable->clear();
    dbo::collection<dbo::ptr<NebulaDenomination>> denominations = session.find<NebulaDenomination>().where("catalogue = ?").where("number = ?")
      .bind(cataloguesCombo->currentText()).bind(catalogueNumber->text());
    for(auto nebula: denominations) {
      WTableRow *row = resultsTable->insertRow(resultsTable->rowCount());
      row->elementAt(0)->addWidget(new WText{nebula->catalogue()});
      row->elementAt(1)->addWidget(new WText{WString("{1}").arg(nebula->number())});
      row->elementAt(2)->addWidget(new WText{nebula->comment()});
      row->elementAt(3)->addWidget(WW<WPushButton>("Add").css("btn btn-primary").onClick([=](WMouseEvent){
        Dbo::Transaction t(session);
        astroSession.modify()->astroSessionObjects().insert(new AstroSessionObject(nebula->ngcObject()));
        t.commit();
        objectsListChanged.emit();
      }));
    }
  };
  catalogueNumber->changed().connect([=](_n1){ searchByCatalogueNumber(); });
  addObjectByCatalogue->addWidget(WW<WContainerWidget>().css("form-inline").add(cataloguesCombo).add(catalogueNumber)
    .add(WW<WPushButton>("Search").css("btn btn-primary").onClick([=](WMouseEvent){ searchByCatalogueNumber(); })));
  addObjectByCatalogue->addWidget(resultsTable);
  q->addTab(addObjectByCatalogue, "Add By Catalogue Number");
}
