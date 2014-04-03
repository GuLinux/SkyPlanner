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

#include "widgets/filterbyconstellation.h"
#include "private/filterbyconstellation_p.h"
#include "utils/d_ptr_implementation.h"
#include <utils/format.h>
#include <Wt/WComboBox>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WLabel>
#include "Wt-Commons/wt_helpers.h"
#include <Wt-Commons/wform.h>
#include "constellationfinder.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByConstellation::Private::Private( FilterByConstellation *q ) : q( q )
{
}

FilterByConstellation::FilterByConstellation( Wt::WContainerWidget *parent ): WCompositeWidget( parent ), d(this)
{
  WComboBox *constellationsCombo = WW<WComboBox>().css("input-sm");
  d->model = new WStandardItemModel(constellationsCombo);
  constellationsCombo->setModel(d->model);
  constellationsCombo->activated().connect([=](int index, _n5){
    d->selected = (index==0) ? ConstellationFinder::Constellation{} : boost::any_cast<ConstellationFinder::Constellation>(d->model->item(index)->data());
    d->changed.emit();
  });
  setImplementation(WW<WContainerWidget>().setInline(true).add(new WLabel(WString("<small>{1}</small>").arg(WString::tr("filter_by_constellation")))).add(constellationsCombo));
  reload();
}

FilterByConstellation::~FilterByConstellation()
{
}

void FilterByConstellation::reload()
{
  d->model->clear();
  d->model->appendRow(new WStandardItem(WString::tr("filter_by_constellation_all")));
  auto allConstellations = ConstellationFinder::constellations();
  vector<ConstellationFinder::Constellation> constellations;
  copy_if(begin(allConstellations), end(allConstellations), back_inserter(constellations), d->filter);
  for(auto constellation: constellations) {
    WStandardItem *item = new WStandardItem(WString::fromUTF8(format("%s (%s)") % constellation.name % constellation.abbrev));
    item->setData(constellation);
    d->model->appendRow(item);
  }
}


void FilterByConstellation::setFilter( const FilterByConstellation::Filter &filter )
{
  d->filter = filter;
  reload();
}


Signal< NoClass > &FilterByConstellation::changed() const
{
  return d->changed;
}
ConstellationFinder::Constellation FilterByConstellation::selectedConstellation() const
{
  return d->selected;
}


