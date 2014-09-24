 /*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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
#include "positiondetailswidget.h"
#include "Wt-Commons/wt_helpers.h"
#include "skyplanner.h"
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WAnchor>
#include <Wt/WImage>
#include <Wt/WTemplate>
#include "utils/format.h"
#include "session.h"
#include "openweather/openweather.h"


using namespace Wt;
using namespace WtCommons;
using namespace std;


PositionDetailsWidget::PositionDetailsWidget(const AstroGroup& astroGroup, const GeoCoder::Place &geoCoderPlace, Session& session, bool showMeteo, Wt::WContainerWidget* parent)
  : WCompositeWidget(parent)
{
  auto astroSession = astroGroup.astroSession();
  auto timezone = astroGroup.timezone;
  WContainerWidget *positionDetails = WW<WContainerWidget>();
  setImplementation(positionDetails);
  Dbo::Transaction t(session);
  astroSession.reread();
  auto addMoonPhaseDetails = [=](const Ephemeris::LunarPhase &lunarPhase) {
    positionDetails->addWidget(new WText(WString::tr("astrosessiontab_moon_phase").arg(static_cast<int>(lunarPhase.illuminated_fraction * 100 ))));
    positionDetails->addWidget(new WBreak);
  };
  if(!astroSession->position()) {
    addMoonPhaseDetails(Ephemeris({}, {}).moonPhase(astroSession->date()));
    return;
  }
//   forecast.fetch(astroSession->position().longitude, astroSession->position().latitude);
  const Ephemeris ephemeris(astroSession->position(), timezone);
  Ephemeris::RiseTransitSet sun = ephemeris.sun(astroSession->date());
  Ephemeris::RiseTransitSet astroTwilight = ephemeris.astronomicalTwilight(astroSession->date());
  Ephemeris::RiseTransitSet moon = ephemeris.moon(astroSession->date());
  Ephemeris::LunarPhase lunarPhase = ephemeris.moonPhase(astroSession->date());
  Ephemeris::Darkness darkness = ephemeris.darknessHours(astroSession->date() );
  if(!geoCoderPlace.formattedAddress.empty()) {
    positionDetails->addWidget(new WText{WString::fromUTF8(geoCoderPlace.formattedAddress)});
    positionDetails->addWidget(new WBreak);
  }
  positionDetails->addWidget(new WText{WString::tr("astrosession_coordinates")
    .arg(WString::fromUTF8(astroSession->position().latitude.printable()) )
    .arg(WString::fromUTF8(astroSession->position().longitude.printable() ) )});
  positionDetails->addWidget(new WBreak);   
  
  positionDetails->addWidget(new WText{WString::tr("printable_timezone_info").arg(WString::fromUTF8(timezone.timeZoneName))});
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_sun_info"))
    .arg(sun.rise.str(DateTime::DateShort) )
    .arg(sun.set.str(DateTime::DateShort) )
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_astro_twilight_info"))
    .arg(astroTwilight.rise.str(DateTime::DateShort))
    .arg(astroTwilight.set.str(DateTime::DateShort))
  ));
  positionDetails->addWidget(new WBreak);
  positionDetails->addWidget(new WText(WString(WString::tr("astrosessiontab_moon_info"))
    .arg(moon.rise.str(DateTime::DateShort))
    .arg(moon.set.str(DateTime::DateShort))
  ));
  positionDetails->addWidget(new WBreak);


  addMoonPhaseDetails(lunarPhase);
  if(lunarPhase.illuminated_fraction <= 0.5) {

    positionDetails->addWidget(new WText{
      WString::tr("astrosessiontab_darkness_hours")
        .arg(darkness.begin.str(DateTime::DateShort))
        .arg(darkness.end.str(DateTime::DateShort))
        .arg(boost::posix_time::to_simple_string(darkness.duration))
    });
  }

  auto now = boost::posix_time::second_clock::local_time();
  if(showMeteo && astroSession->when() > now /* && astroSession->when() - now < boost::posix_time::hours(72) */) {
    positionDetails->addWidget(new WBreak);
    WContainerWidget *weatherWidget = WW<WContainerWidget>().css("container");
    WContainerWidget *weatherWidgetRow = WW<WContainerWidget>().css("row");
    OpenWeather openWeather;
    auto forecast = openWeather.forecast(astroSession->position(), "");
    for(auto weather: forecast.weathers()) {
        WTemplate *weatherItemWidget = new WTemplate(WString::tr("weather_item"));
        weatherItemWidget->bindString("weather-date", WDateTime::fromPosixTime(weather->dateGMT()).toString("dd/MM"));
        weatherWidgetRow->addWidget(weatherItemWidget);
    }
    positionDetails->addWidget(weatherWidget);
  }
}

PositionDetailsWidget::~PositionDetailsWidget()
{
}
