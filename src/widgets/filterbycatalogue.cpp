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

#include "widgets/filterbycatalogue.h"
#include "private/filterbycatalogue_p.h"
#include "utils/d_ptr_implementation.h"
#include <utils/format.h>
#include <Wt/WComboBox>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WLabel>
#include "wt_helpers.h"
#include <wform.h>
#include "models/Models"

using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByCatalogue::Private::Private( Session &session, FilterByCatalogue *q ) : session(session), q( q )
{
}

FilterByCatalogue::FilterByCatalogue( Session &session, Wt::WContainerWidget *parent ): WCompositeWidget( parent ), d(session, this)
{
  d->cataloguesCombo = WW<WComboBox>().css("input-sm");
  d->model = new WStandardItemModel(d->cataloguesCombo);
  d->cataloguesCombo->setModel(d->model);
  d->cataloguesCombo->activated().connect([=](int index, _n5) { d->changed.emit(); });

  d->model->clear();
  d->model->appendRow(new WStandardItem(WString::tr("filter_by_catalogue_all")));
  Dbo::Transaction t(session);
  for(auto catalogue: session.find<Catalogue>().where("hidden = ?").bind(Catalogue::Visible).orderBy("priority ASC").resultList() ) {
    WStandardItem *item = new WStandardItem(WString::fromUTF8(catalogue->name() ));
    item->setData(catalogue);
    d->model->appendRow(item);
  }

  setImplementation(WW<WContainerWidget>().setInline(true).add(new WLabel(WString("<small>{1}</small>").arg(WString::tr("filter_by_catalogue")))).add(d->cataloguesCombo));
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
  return d->cataloguesCombo->currentIndex()==0 ? CataloguePtr{} : boost::any_cast<CataloguePtr>(d->model->item(d->cataloguesCombo->currentIndex())->data() );;
}

void FilterByCatalogue::resetDefaultValue()
{
  d->cataloguesCombo->setCurrentIndex(0);
  d->changed.emit();
}

