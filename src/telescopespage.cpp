/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#include "telescopespage.h"
#include "private/telescopespage_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/format.h"
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include "Models"
#include <Wt/Auth/Login>
#include <Wt/WTable>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <Wt/WSpinBox>
#include <Wt/WLabel>
#include <boost/format.hpp>

using namespace Wt;
using namespace WtCommons;
using namespace std;

TelescopesPage::Private::Private( Session &session, TelescopesPage *q ) : session(session), q( q )
{
}

TelescopesPage::~TelescopesPage()
{
}

TelescopesPage::TelescopesPage( Session &session, WContainerWidget *parent )
  : WContainerWidget(parent), d( session, this )
{
  WLineEdit *telescopeName = WW<WLineEdit>();
  WSpinBox *telescopeDiameter = WW<WSpinBox>();
  WSpinBox *telescopeFocalLength = WW<WSpinBox>();
  WLabel *telescopeNameLabel = WW<WLabel>("Telescope Name").css("control-label");
  WLabel *telescopeDiameterLabel = WW<WLabel>("Diameter (mm)").css("control-label");
  WLabel *telescopeFocalLengthLabel = WW<WLabel>("Focal Length (mm)").css("control-label");
  
  telescopeNameLabel->setBuddy(telescopeName);
  telescopeDiameterLabel->setBuddy(telescopeDiameter);
  telescopeFocalLengthLabel->setBuddy(telescopeFocalLength);
  
  telescopeName->setEmptyText("Telescope Name");
  telescopeDiameter->setEmptyText("Diameter");
  telescopeFocalLength->setEmptyText("Focal Length");
  WPushButton *addTelescopeButton = WW<WPushButton>("Add Telescope").css("btn btn-primary").onClick([=](WMouseEvent){
    Dbo::Transaction t(d->session);
    d->session.user().modify()->telescopes().insert(new Telescope(telescopeName->text().toUTF8(), telescopeDiameter->value(), telescopeFocalLength->value()));
    t.commit();
    d->populate();
  });
  addWidget(WW<WContainerWidget>().css("form-horizontal")
  .add(WW<WContainerWidget>().css("control-group")
    .add(telescopeNameLabel)
    .add(WW<WContainerWidget>().css("controls").add(telescopeName))
  )
  .add(WW<WContainerWidget>().css("control-group")
    .add(telescopeDiameterLabel)
    .add(WW<WContainerWidget>().css("controls").add(telescopeDiameter))
  )
  .add(WW<WContainerWidget>().css("control-group")
    .add(telescopeFocalLengthLabel)
    .add(WW<WContainerWidget>().css("controls").add(telescopeFocalLength))
  )
  .add(WW<WContainerWidget>().css("control-group")
    .add(WW<WContainerWidget>().css("controls").add(addTelescopeButton))
   )
    
  );
  d->telescopesTable = WW<WTable>(this).addCss("table table-striped table-hover");
  d->telescopesTable->setHeaderCount(1);
  d->loginChanged();
  session.login().changed().connect(bind(&Private::loginChanged, d.get()));
}

void TelescopesPage::Private::populate()
{
  Dbo::Transaction t(session);
  telescopesTable->clear();
  telescopesTable->elementAt(0, 0)->addWidget(new WText{"Name"});
  telescopesTable->elementAt(0, 1)->addWidget(new WText{"Diameter (mm)"});
  telescopesTable->elementAt(0, 2)->addWidget(new WText{"Focal Length (mm)"});
  telescopesTable->elementAt(0, 3)->addWidget(new WText{"Magnitude gain"});
  telescopesTable->elementAt(0, 4)->addWidget(new WText{"Magnitude limit (naked eye=6)"});
  for(auto telescope: session.user()->telescopes()) {
    WTableRow *row = telescopesTable->insertRow(telescopesTable->rowCount());
    row->elementAt(0)->addWidget(new WText{telescope->name() });
    row->elementAt(1)->addWidget(new WText{WString("{1}").arg(telescope->diameter()) });
    row->elementAt(2)->addWidget(new WText{WString("{1}").arg(telescope->focalLength()) });
    row->elementAt(3)->addWidget(new WText{ format("%.3f") % telescope->limitMagnitudeGain() });
    row->elementAt(4)->addWidget(new WText{ format("%.3f") % (telescope->limitMagnitudeGain() + 6) });
    row->elementAt(5)->addWidget(WW<WPushButton>("Delete").css("btn btn-danger").onClick([=](WMouseEvent){
      Dbo::Transaction t(session);
      session.user().modify()->telescopes().erase(telescope);
      t.commit();
      populate();
    }) );
  }
}

void TelescopesPage::Private::loginChanged()
{
  bool loggedIn = session.login().loggedIn();
  if(!loggedIn) {
    telescopesTable->clear();
    q->disable();
    return;
  }
  q->enable();
  populate();
}

