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

#include "astroobjectstable.h"
#include "private/astroobjectstable_p.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt-Commons/wt_helpers.h>
#include "models/Models"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroObjectsTable::Private::Private(AstroObjectsTable *q) : q(q)
{
}

AstroObjectsTable::AstroObjectsTable(WContainerWidget *parent)
  : WCompositeWidget(parent), d(this)
{
  d->objectsTable = WW<WTable>().addCss("table table-hover astroobjects-table");
  setImplementation(WW<WContainerWidget>().add(WW<WContainerWidget>().addCss("table-responsive").add(d->objectsTable)));
}

void AstroObjectsTable::Private::header()
{
  objectsTable->clear();
  objectsTable->elementAt(0,0)->addWidget(new WText{WString::tr("object_column_names")});
  objectsTable->elementAt(0,1)->addWidget(new WText{WString::tr("object_column_type")});
  objectsTable->elementAt(0,2)->addWidget(new WText{WString::tr("object_column_ar")});
  objectsTable->elementAt(0,3)->addWidget(new WText{WString::tr("object_column_dec")});
  objectsTable->elementAt(0,4)->addWidget(new WText{WString::tr("object_column_constellation")});
  objectsTable->elementAt(0,5)->addWidget(new WText{WString::tr("object_column_angular_size")});
  objectsTable->elementAt(0,6)->addWidget(new WText{WString::tr("object_column_magnitude")});
  objectsTable->elementAt(0,7)->addWidget(new WText{WString::tr("object_column_highest_time")});
  objectsTable->elementAt(0,8)->addWidget(new WText{WString::tr("object_column_max_altitude")});
  objectsTable->elementAt(0,9)->addWidget(new WText{WString::tr("object_column_difficulty")});
}

void AstroObjectsTable::populate(const vector<Object> &objects, Dbo::Transaction &transaction)
{
  d->header();
/*
  for(auto object: objects) {
    WTableRow *row = objectsTable->insertRow(objectsTable->rowCount());
    if(addedObject == sessionObject) {
      objectAddedRow = row;
      row->addStyleClass("success");
    }
    row->elementAt(0)->addWidget(WW<ObjectNamesWidget>(new ObjectNamesWidget{sessionObject, timezone, selectedTelescope, session}).setInline(true).onClick([=](WMouseEvent){
      if(selectedRow)
        selectedRow->removeStyleClass("info");
      if(objectAddedRow)
        objectAddedRow->removeStyleClass("success");
      row->addStyleClass("info");
      selectedRow = row;
    }));
    row->elementAt(1)->addWidget(new WText{sessionObject->ngcObject()->typeDescription() });
    row->elementAt(2)->addWidget(new WText{ Utils::htmlEncode( sessionObject->coordinates().rightAscension.printable(Angle::Hourly) ) });
    row->elementAt(3)->addWidget(new WText{ Utils::htmlEncode( WString::fromUTF8( sessionObject->coordinates().declination.printable() )) });
    row->elementAt(4)->addWidget(new WText{ WString::fromUTF8(sessionObject->ngcObject()->constellation().name) });
    row->elementAt(5)->addWidget(new WText{ Utils::htmlEncode( WString::fromUTF8( Angle::degrees(sessionObject->ngcObject()->angularSize()).printable() )) });
    row->elementAt(6)->addWidget(new WText{ sessionObject->ngcObject()->magnitude() > 90. ? "N/A" : (format("%.1f") % sessionObject->ngcObject()->magnitude()).str() });
    auto bestAltitude = sessionObject->bestAltitude(ephemeris, 1);
    row->elementAt(7)->addWidget(new WText{ bestAltitude.when.str() });
    row->elementAt(8)->addWidget(new WText{ Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) });
    row->elementAt(9)->addWidget(new ObjectDifficultyWidget{sessionObject->ngcObject(), selectedTelescope, bestAltitude.coordinates.altitude.degrees() }); 
    

       
    #define OBJECTS_TABLE_COLS 11

    WTableRow *astroObjectRow = objectsTable->insertRow(objectsTable->rowCount());
    WTableCell *astroObjectCell = astroObjectRow->elementAt(0);
    astroObjectCell->setHidden(true);
    astroObjectCell->setColumnSpan(OBJECTS_TABLE_COLS);
    WPushButton *toggleMoreInfo = WW<WPushButton>(row->elementAt(0)).css("btn btn-xs pull-right hidden-print").setTextFormat(XHTMLUnsafeText).setText("&#x25bc;").setAttribute("title", WString::tr("astroobject_extended_info_title").toUTF8() );
    auto showHideMoreInfo = [=] {
      toggleMoreInfo->setText(!astroObjectCell->isVisible() ? "&#x25b2;" : "&#x25bc;");
      toggleMoreInfo->toggleStyleClass("active", !astroObjectCell->isVisible());
      if(astroObjectCell->isVisible()) {
        astroObjectCell->clear();
        astroObjectCell->setHidden(true);
        return;
      }
      astroObjectCell->setHidden(false);
      astroObjectCell->clear();
      astroObjectCell->addWidget(new AstroObjectWidget(sessionObject, session, timezone, selectedTelescope, {}, {WW<WPushButton>(WString::tr("buttons_close")).css("btn-xs").onClick([=](WMouseEvent){
        astroObjectCell->clear();
        astroObjectCell->setHidden(true);
        toggleMoreInfo->removeStyleClass("active");
        toggleMoreInfo->setText("&#x25bc");
      }) } ));
    };
    toggleMoreInfo->clicked().connect(std::bind(showHideMoreInfo));
  }
*/
}
