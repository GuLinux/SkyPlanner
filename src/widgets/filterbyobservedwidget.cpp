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
#include "filterbyobservedwidget.h"
#include "private/filterbyobservedwidget_p.h"

#include <utils/d_ptr_implementation.h>
#include "wt_helpers.h"
#include <Wt/WLabel>

using namespace Wt;
using namespace WtCommons;
using namespace std;

FilterByObservedWidget::Private::Private(FilterByObservedWidget *q): q(q)
{
}

FilterByObservedWidget::FilterByObservedWidget(const Wt::WString &label, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), d(this)
{
  d->combo = WW<WComboBox>().css("input-sm");
  d->combo->addItem(WString::tr("observed_any"));
  d->combo->addItem(WString::tr("observed_no"));
  d->combo->addItem(WString::tr("observed_yes"));
  setImplementation(WW<WContainerWidget>().setInline(true).add(new WLabel{label}).add(d->combo));
  d->combo->activated().connect([=](int,_n5){ d->changed.emit(); });
}

FilterByObservedWidget::~FilterByObservedWidget()
{
}

void FilterByObservedWidget::resetDefaultValue()
{
  d->combo->setCurrentIndex(0);
  d->changed.emit();
}

boost::logic::tribool FilterByObservedWidget::value() const {
  if(d->combo->currentIndex() == 0)
    return boost::logic::indeterminate;
  return d->combo->currentIndex() != 1;
}

Signal<> &FilterByObservedWidget::changed() const
{
  return d->changed;
}
