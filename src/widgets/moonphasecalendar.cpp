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
#include <Wt/WDate>
#include <Wt/WLocalDateTime>
#include <Wt/WTime>
#include <libnova/libnova.h>
#include <boost/format.hpp>
#include "wt_helpers.h"
using namespace std;
using namespace Wt;
using namespace WtCommons;

class MoonPhaseCalendar::Private {
public:
    Private(MoonPhaseCalendar *q);
    class DayPhaseWidget : public WContainerWidget {
    public:
        DayPhaseWidget(WDate date, WText *day_text, WContainerWidget* parent = 0);
	WText *day_text;
        DayPhaseWidget *update(const WDate &date);
	WImage *image;
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
  : WContainerWidget(parent), day_text{day_text}
{
  setStyleClass("moon-calendar-cell");
  image = new WImage();
  day_text->addStyleClass("moon-calendar-text");
  addWidget(day_text);
  image->addStyleClass("moon-calendar-image");
  addWidget( image );
  update(date);

}

MoonPhaseCalendar::Private::DayPhaseWidget* MoonPhaseCalendar::Private::DayPhaseWidget::update(const WDate& date)
{
  day_text->addStyleClass("moon-calendar-text");
  auto moonPhaseAngle = ln_get_lunar_phase(date.toJulianDay());
  bool is_crescent = ln_get_lunar_bright_limb(date.toJulianDay()) > 180;
  double phase = (is_crescent ? 180. - moonPhaseAngle : 180. + moonPhaseAngle) / 360.;
  image->setImageLink(URLs::moon_phase_image(phase));
  return this;
}





WWidget* MoonPhaseCalendar::renderCell(WWidget* widget, const WDate& date)
{
  auto cell = dynamic_cast<Private::DayPhaseWidget*>(widget);
  if(cell) {
    WCalendar::renderCell(cell->day_text, date);
    return cell->update(date);
  }
  return new Private::DayPhaseWidget(date, reinterpret_cast<WText*>(WCalendar::renderCell(nullptr, date)));
}



class MoonPhaseCalendar::Picker::Private {
public:
  Private(const WDate &date);
  WDate date;
  WDialog *dialog;
  MoonPhaseCalendar *calendar;
  WPushButton *button;
  WText *label;
  void update_label();
};

MoonPhaseCalendar::Picker::Private::Private(const WDate& date) 
  : date{ date.isNull() ? WDate::currentDate() : date },
    dialog{new WDialog{WString::tr("set_date")}},
    calendar{new MoonPhaseCalendar{dialog->contents()}},
    button{new WPushButton{WString::tr("set_date")}},
    label{new WText}
{
//   button->setIcon("/resources/themes/bootstrap/3/date-edit-button.png"); // TODO: proper url?
  dialog->setClosable(true);
  calendar->select(date);
  calendar->clicked().connect([this](const WDate &date, _n5) {
    this->date = date;
    update_label();
    dialog->hide();
  });
  update_label();
}

void MoonPhaseCalendar::Picker::Private::update_label()
{
  label->setText(WLocalDateTime(date, {0,0,0}).toString("dddd, dd MMMM"));
}



MoonPhaseCalendar::Picker::Picker(const WDate& date, WContainerWidget* parent): WCompositeWidget(parent), dptr(date)
{
  auto container = WW<WContainerWidget>().add(d->label).add(d->button);
  setImplementation(container.get());
  d->button->clicked().connect(bind([=]{d->dialog->show();}));
}


WDate MoonPhaseCalendar::Picker::date() const
{
  return d->date;
}

void MoonPhaseCalendar::Picker::setDate(const WDate& date)
{
  d->date = date;
  d->update_label();
  d->calendar->select(date);
}



