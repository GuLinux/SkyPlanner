#include "weatherwidget.h"
#include "openweather/openweather.h"
#include <Wt-Commons/wt_helpers.h>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WDateTime>
#include "utils/format.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
WeatherWidget::WeatherWidget(const Coordinates::LatLng latlng, const GeoCoder::Place &place, const boost::posix_time::ptime &date, Format widgetFormat, WContainerWidget *parent)
    : WCompositeWidget(parent)
{
    // TODO: translate
    WContainerWidget *weatherWidgetRow = WW<WContainerWidget>().css("row");
    WContainerWidget *weatherWidget = WW<WContainerWidget>().css("container").add(weatherWidgetRow);
    OpenWeather openWeather;
    boost::posix_time::time_duration difference(date - boost::posix_time::second_clock().local_time());
    auto forecast = openWeather.forecast(latlng, format("%s, %s") % place.city % place.country, difference.total_seconds() / 60 / 60 /24);
    if(!forecast) {
        setImplementation(new WText{"Weather unavailable"});
        return;
    }
    for(Weather weather: forecast->weathers()) {
        WTemplate *weatherItemWidget = new WTemplate(WString::tr("weather_item"));
        weatherItemWidget->bindString("weather_date", WDateTime::fromPosixTime(weather.dateGMT()).toString("dd/MM"));
        weatherItemWidget->bindString("weather_name", WString::fromUTF8(weather.summaries()[0].main()));
        weatherItemWidget->bindString("weather_description", WString::fromUTF8(weather.summaries()[0].description()));
        weatherItemWidget->bindString("weather_icon_url", WString::fromUTF8(weather.summaries()[0].iconURL()));

        weatherItemWidget->bindInt("clouds", weather.clouds());
        weatherItemWidget->bindInt("humidity", weather.humidity());
        weatherItemWidget->bindString("temp_min", format("%.1f") % weather.temperature().min().celsius());
        weatherItemWidget->bindString("temp_max", format("%.1f") % weather.temperature().max().celsius());
        weatherWidgetRow->addWidget(weatherItemWidget);
    }
    setImplementation(weatherWidget);
}
