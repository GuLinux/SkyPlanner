/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Marco Gulino <email>
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

#include "widgets/filterbycatalogue.h"
#include "private/filterbycatalogue_p.h"
#include "utils/d_ptr_implementation.h"
#include <utils/format.h>
#include <Wt/WComboBox>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WLabel>
#include "Wt-Commons/wt_helpers.h"
#include <Wt-Commons/wform.h>
#include "models/Models"

using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByCatalogue::Private::Private( Session &session, FilterByCatalogue *q ) : session(session), q( q )
{
}

FilterByCatalogue::FilterByCatalogue( Session &session, Wt::WContainerWidget *parent ): WCompositeWidget( parent ), d(session, this)
{
  WComboBox *cataloguesCombo = WW<WComboBox>();
  d->model = new WStandardItemModel(cataloguesCombo);
  cataloguesCombo->setModel(d->model);
  cataloguesCombo->activated().connect([=](int index, _n5){
    d->selected = index==0 ? CataloguePtr() : boost::any_cast<CataloguePtr>(d->model->item(index)->data() );
    d->changed.emit();
  });

  d->model->clear();
  d->model->appendRow(new WStandardItem(WString::tr("filter_by_catalogue_all")));
  Dbo::Transaction t(session);
  for(auto catalogue: session.find<Catalogue>().where("hidden = ?").bind(false).orderBy("priority ASC").resultList() ) {
    WStandardItem *item = new WStandardItem(WString::fromUTF8(catalogue->name() ));
    item->setData(catalogue);
    d->model->appendRow(item);
  }

  setImplementation(WW<WContainerWidget>().setInline(true).add(new WLabel(WString::tr("filter_by_catalogue"))).add(cataloguesCombo));
}

FilterByCatalogue::~FilterByCatalogue()
{
}


Signal< NoClass > &FilterByCatalogue::changed() const
{
  return d->changed;
}
CataloguePtr FilterByCatalogue::selectedCatalogue() const
{
  return d->selected;
}


