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
  WComboBox *constellationsCombo = WW<WComboBox>();
  WStandardItemModel *model = new WStandardItemModel(constellationsCombo);
  model->appendRow(new WStandardItem(WString::tr("filter_by_constellation_all")));
  for(auto constellation: ConstellationFinder::constellations()) {
    WStandardItem *item = new WStandardItem(WString::fromUTF8(format("%s (%s)") % constellation.name % constellation.abbrev));
    item->setData(constellation);
    model->appendRow(item);
  }
  constellationsCombo->setModel(model);
  constellationsCombo->activated().connect([=](int index, _n5){
    if(index==0)
      d->selected = ConstellationFinder::Constellation();
    else {
      d->selected = boost::any_cast<ConstellationFinder::Constellation>(model->item(index)->data());
    };
    d->changed.emit();
  });
  setImplementation(WW<WContainerWidget>().setInline(true).add(new WLabel(WString::tr("filter_by_constellation"))).add(constellationsCombo));
}

FilterByConstellation::~FilterByConstellation()
{
}

Signal< NoClass > &FilterByConstellation::changed() const
{
  return d->changed;
}
ConstellationFinder::Constellation FilterByConstellation::selectedConstellation() const
{
  return d->selected;
}


