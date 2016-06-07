/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
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

#include "moonphasecalendar.h"
#include <urls.h>
#include <Wt/WContainerWidget>
#include <Wt/WDialog>
#include <Wt/WPushButton>
#include <Wt/WImage>
#include <libnova/libnova.h>
#include <boost/format.hpp>

using namespace std;
using namespace Wt;

class MoonPhaseCalendar::Private {
public:
    Private(MoonPhaseCalendar *q);
    class DayPhaseWidget : public WContainerWidget {
    public:
        DayPhaseWidget(WDate date, WText *day_text, WContainerWidget* parent = 0);
	WDate date;
	WText *day_text;
    };
private:
    MoonPhaseCalendar *q;
};

MoonPhaseCalendar::Private::Private(MoonPhaseCalendar* q) : q(q)
{
}


MoonPhaseCalendar::MoonPhaseCalendar(WContainerWidget* parent): WCalendar(parent), dptr(this)
{
}


MoonPhaseCalendar::~MoonPhaseCalendar()
{
}

MoonPhaseCalendar::Private::DayPhaseWidget::DayPhaseWidget(WDate date, WText* day_text, WContainerWidget* parent)
  : WContainerWidget(parent), date{date}, day_text{day_text}
{
  setStyleClass("moon-calendar-cell");
  day_text->addStyleClass("moon-calendar-text");
  auto moonPhaseAngle = ln_get_lunar_phase(date.toJulianDay());
  bool is_crescent = ln_get_lunar_bright_limb(date.toJulianDay()) > 180;
  double phase = (is_crescent ? 180. - moonPhaseAngle : 180. + moonPhaseAngle) / 360.;
  addWidget(day_text);
  auto image = new WImage( URLs::moon_phase_image(phase));
  image->addStyleClass("moon-calendar-image");
  addWidget( image );
}


WWidget* MoonPhaseCalendar::renderCell(WWidget* widget, const WDate& date)
{
//   if(widget)
//     return widget;
  return new Private::DayPhaseWidget(date, reinterpret_cast<WText*>(WCalendar::renderCell(widget, date)));
}


WDialog* MoonPhaseCalendar::dialog(WObject *parent)
{
  WDialog *dialog = new WDialog();
  MoonPhaseCalendar *calendar = new MoonPhaseCalendar(dialog->contents());
  dialog->setClosable(true);
  return dialog;
}

WPushButton* MoonPhaseCalendar::button(WContainerWidget* parent, const WString& text)
{
  WPushButton *button = new WPushButton(text, parent);
  auto popup = dialog(button);
  button->clicked().connect(bind([=]{popup->show();}));
  return button;
}

