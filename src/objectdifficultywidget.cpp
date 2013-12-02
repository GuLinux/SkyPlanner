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

#include "Models"
#include "objectdifficultywidget.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/format.h"
#include <Wt/WText>
#include "utils/d_ptr_implementation.h"


using namespace WtCommons;
using namespace Wt;
using namespace std;

class ObjectDifficultyWidget::Private {
public:
  Private(ObjectDifficultyWidget *q) : q(q) {}
private:
  ObjectDifficultyWidget * const q;
};

ObjectDifficultyWidget::~ObjectDifficultyWidget()
{

}

ObjectDifficultyWidget::ObjectDifficultyWidget( const Wt::Dbo::ptr< AstroSessionObject > &object, const Wt::Dbo::ptr< Telescope > &telescope, double maxAltitudeInDegrees, Wt::WContainerWidget *parent )
  : WContainerWidget(parent), d(this)
{
  int difficulty = object->difficulty(telescope);
  if(maxAltitudeInDegrees < 10.) {
    addWidget(new WText{"Too low"});
    return;
  }
  string difficultyText;
  difficulty = difficulty > 0 ? difficulty/25 : -1;
  switch(difficulty) {
    case -1:
      difficultyText = "N/A";
      break;
    case 0:
      difficultyText = "Very easy";
      break;
    case 1:
      difficultyText = "Easy";
      break;
    case 2:
      difficultyText = "Average";
      break;
    case 3:
      difficultyText = "Difficult";
      break;
    case 4:
      difficultyText = "Out of range";
      break;
  } 
  addWidget(new WText{difficultyText});
}
