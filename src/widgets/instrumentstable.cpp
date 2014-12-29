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

#include "instrumentstable.h"
#include <Wt/WContainerWidget>
#include <Wt/WTable>
#include <Wt/WTableRow>
#include <Wt/WText>
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include <Wt-Commons/wglyphicon.h>
#include "utils/format.h"
#include "Models"

using namespace std;
using namespace Wt;
using namespace WtCommons;

InstrumentsTable::~InstrumentsTable()
{

}

InstrumentsTable::InstrumentsTable(const UserPtr &user, Session& session, WContainerWidget* parent)
  : WCompositeWidget(parent), user(user), session(session)
{
  instrumentsTable = WW<WTable>().addCss("table table-condensed table-hover table-bordered");
  setImplementation(instrumentsTable);
  reload();
}

void InstrumentsTable::reload()
{
  instrumentsTable->clear();
  Dbo::Transaction t(session);
  instrumentsTable->setHeaderCount(1);
  instrumentsTable->elementAt(0, 0)->addWidget(WW<WText>(WString::tr("telescope")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 1)->addWidget(WW<WText>(WString::tr("telescope_dia_focal")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 2)->addWidget(WW<WText>(WString::tr("focalMultiplier")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 3)->addWidget(WW<WText>(WString::tr("eyepiece")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 4)->addWidget(WW<WText>(WString::tr("eyepiece_focal_length")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 5)->addWidget(WW<WText>(WString::tr("afov")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 6)->addWidget(WW<WText>(WString::tr("magnification")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 7)->addWidget(WW<WText>(WString::tr("fov")).css("printable-small-text") );
  instrumentsTable->elementAt(0, 8)->addStyleClass("hidden-print");
  instrumentsTable->elementAt(0, 8)->addWidget(WW<WGlyphicon>("glyphicon-refresh").addCss("link").onClick([=](WMouseEvent){ reload(); }).get()->color("blue") );
  
  for(auto telescope: user->telescopes()) {
    for(auto focalMultiplier: user->focalModifiers("no multiplier")) {
      WString focalMultiplierLabel = focalMultiplier->ratio() == 1. ?
	WString(focalMultiplier->name()) :
	WString("{1} ({2}x)").arg(focalMultiplier->name()).arg((::format("%.2f") % focalMultiplier->ratio()).str() );
      for(auto eyepiece: user->eyepieces()) {
	WTableRow *row = instrumentsTable->insertRow(instrumentsTable->rowCount());
	OpticalSetup setup(telescope, eyepiece, focalMultiplier);
	row->elementAt(0)->addWidget(WW<WText>(WString::fromUTF8(telescope->name())).css("printable-small-text") );
	row->elementAt(1)->addWidget(WW<WText>(format("%dmm/%dmm") % telescope->diameter() % telescope->focalLength() ).css("printable-small-text") );
	row->elementAt(2)->addWidget(WW<WText>(focalMultiplierLabel).css("printable-small-text") );
	row->elementAt(3)->addWidget(WW<WText>(WString::fromUTF8(eyepiece->name())).css("printable-small-text") );
	row->elementAt(4)->addWidget(WW<WText>(format("%dmm") % eyepiece->focalLength() ).css("printable-small-text") );
	row->elementAt(5)->addWidget(WW<WText>(WString::fromUTF8(eyepiece->aFOV().printable(Angle::IntDegrees))).css("printable-small-text") );
	row->elementAt(6)->addWidget(WW<WText>(format("%.2f") % setup.magnification()).css("printable-small-text") );
	row->elementAt(7)->addWidget(WW<WText>(WString::fromUTF8(setup.fov().printable())).css("printable-small-text") );
	row->elementAt(8)->addWidget(WW<WGlyphicon>("glyphicon-remove").addCss("link").onClick([=](WMouseEvent){ row->hide(); }).get()->color("red"));
	row->elementAt(8)->addStyleClass("hidden-print");
      }
    }
  }
}

