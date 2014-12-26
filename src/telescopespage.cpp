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
#include <Wt/WCheckBox>
#include <Wt/WToolBar>
#include <Wt-Commons/wform.h>
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
  addStyleClass("container");
  d->setupTelescopesTable();
  d->loginChanged();
  session.login().changed().connect(bind(&Private::loginChanged, d.get()));
}


void TelescopesPage::Private::setupTelescopesTable()
{
  WLineEdit *telescopeName = WW<WLineEdit>().css("input-sm");
  WSpinBox *telescopeDiameter = WW<WSpinBox>().css("input-sm");
  WSpinBox *telescopeFocalLength = WW<WSpinBox>().css("input-sm");
  WLabel *telescopeNameLabel = WW<WLabel>(WString::tr("telescopes_telescope_name")).css("control-label");
  WLabel *telescopeDiameterLabel = WW<WLabel>(WString::tr("telescopes_diameter_mm")).css("control-label");
  WLabel *telescopeFocalLengthLabel = WW<WLabel>(WString::tr("telescopes_focal_length_mm")).css("control-label");
  
  telescopeName->setTextSize(38);
  telescopeDiameter->setTextSize(8);
  telescopeFocalLength->setTextSize(8);
  
  telescopeNameLabel->setBuddy(telescopeName);
  telescopeDiameterLabel->setBuddy(telescopeDiameter);
  telescopeFocalLengthLabel->setBuddy(telescopeFocalLength);
  isDefault = WW<WCheckBox>(WString::tr("buttons_default")).addCss("checkbox-no-form-control");

  telescopeName->setEmptyText(WString::tr("telescopes_telescope_name"));
  telescopeDiameter->setEmptyText(WString::tr("telescopes_diameter_mm"));
  telescopeFocalLength->setEmptyText(WString::tr("telescopes_focal_length_mm"));
  WPushButton *addTelescopeButton = WW<WPushButton>(WString::tr("buttons_add")).css("btn btn-primary").onClick([=](WMouseEvent){
    Dbo::Transaction t(session);
    if(isDefault->isChecked())
      session.execute(R"(UPDATE telescope set "default" = ? where "user_id" = ?)").bind(false).bind(session.user().id());
    session.user().modify()->telescopes().insert(new Telescope(telescopeName->text().toUTF8(), telescopeDiameter->value(), telescopeFocalLength->value(), isDefault->isChecked() ));
    t.commit();
    changed.emit();
    populateTelescopes();
  });
  q->addWidget(WW<WContainerWidget>().addCss("row").add(WW<WForm>(WForm::Inline).get()
    ->add(telescopeName, "telescopes_telescope_name")
    ->add(telescopeDiameter, "telescopes_diameter_mm")
    ->add(telescopeFocalLength, "telescopes_focal_length_mm")
    ->add(isDefault)
    ->addButton(addTelescopeButton))
  );
  telescopesTable = WW<WTable>().addCss("table table-striped table-hover");
  telescopesTable->setHeaderCount(1);
  q->addWidget(WW<WContainerWidget>().addCss("row").add(telescopesTable));
}


void TelescopesPage::Private::populateTelescopes()
{
  Dbo::Transaction t(session);
  telescopesTable->clear();
  bool isOnlyTelescope = session.user()->telescopes().size() == 0;
  isDefault->setChecked(isOnlyTelescope);
  isDefault->setHidden(isOnlyTelescope);

  telescopesTable->elementAt(0, 0)->addWidget(new WText{WString::tr("telescopes_telescope_name")});
  telescopesTable->elementAt(0, 1)->addWidget(new WText{WString::tr("telescopes_diameter_mm")});
  telescopesTable->elementAt(0, 2)->addWidget(new WText{WString::tr("telescopes_focal_length_mm")});
  telescopesTable->elementAt(0, 3)->addWidget(new WText{WString::tr("telescopes_magnitude_gain")});
  telescopesTable->elementAt(0, 4)->addWidget(new WText{WString::tr("telescopes_magnitude_limit_naked")});
  for(auto telescope: session.user()->telescopes()) {
    WPushButton *defaultButton = WW<WPushButton>(WString::tr("buttons_default")).css("btn-xs").setEnabled(!telescope->isDefault()).addCss(telescope->isDefault() ? "btn-success" : "btn-primary").onClick([=](WMouseEvent){
      if(telescope->isDefault()) return;
        Dbo::Transaction t(session);
        for(TelescopePtr current: session.user()->telescopes()) {
          current.modify()->setDefault(telescope.id() == current.id());
          current.flush();
        }
        t.commit();
        populateTelescopes();
    });
    
    WTableRow *row = telescopesTable->insertRow(telescopesTable->rowCount());
    row->elementAt(0)->addWidget(new WText{telescope->name() });
    row->elementAt(1)->addWidget(new WText{WString("{1}").arg(telescope->diameter()) });
    row->elementAt(2)->addWidget(new WText{WString("{1}").arg(telescope->focalLength()) });
    row->elementAt(3)->addWidget(new WText{ format("%.3f") % telescope->limitMagnitudeGain() });
    row->elementAt(4)->addWidget(new WText{ format("%.3f") % (telescope->limitMagnitudeGain() + 6) });
    row->elementAt(5)->addWidget(
      WW<WToolBar>()
      .addButton(defaultButton)
      .addButton(WW<WPushButton>(WString::tr("buttons_remove")).css("btn btn-danger btn-xs").onClick([=](WMouseEvent){
        Dbo::Transaction t(session);
        session.user().modify()->telescopes().erase(telescope);
        Dbo::ptr<Telescope> tel = telescope;
        tel.remove();
        t.commit();
        changed.emit();
        populateTelescopes();
      }) )
    );
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
  populateTelescopes();
}

Signal<> &TelescopesPage::changed() const
{
  return d->changed;
}
