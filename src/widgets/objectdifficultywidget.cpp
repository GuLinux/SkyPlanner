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
  bool hasDifficulty{true};
private:
  ObjectDifficultyWidget * const q;
};

ObjectDifficultyWidget::~ObjectDifficultyWidget()
{

}

ObjectDifficultyWidget::ObjectDifficultyWidget( const Wt::Dbo::ptr< NgcObject > &object, const Wt::Dbo::ptr< Telescope > &telescope, double maxAltitudeInDegrees, Wt::WContainerWidget *parent )
  : WContainerWidget(parent), d(this)
{
  setInline(true);
  int difficulty = object->difficulty(telescope);
  if(maxAltitudeInDegrees < 10.) {
    addWidget(new WText{WString::tr("objectdifficulty_too_low")});
    return;
  }
  WString difficultyText;
  difficulty = difficulty > 0 ? difficulty/20 : -1;
  switch(difficulty) {
    case -1:
      difficultyText = WString::tr("objectdifficulty_n_a");
      d->hasDifficulty = false;
      break;
    case 0:
      difficultyText = WString::tr("objectdifficulty_very_easy");
      break;
    case 1:
      difficultyText = WString::tr("objectdifficulty_easy");
      break;
    case 2:
      difficultyText = WString::tr("objectdifficulty_average");
      break;
    case 3:
      difficultyText = WString::tr("objectdifficulty_difficult");
      break;
    case 4:
      difficultyText = WString::tr("objectdifficulty_very_difficult");
      break;
    default:
      difficultyText = WString::tr("objectdifficulty_out_of_range");
      break;
  } 
  addWidget(new WText{difficultyText});
}

bool ObjectDifficultyWidget::hasDifficulty() const
{
  return d->hasDifficulty;
}
