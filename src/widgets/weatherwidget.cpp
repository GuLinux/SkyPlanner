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
WeatherWidget::WeatherWidget(const Coordinates::LatLng latlng, const string &cityName, const boost::posix_time::ptime &date, WContainerWidget *parent)
    : WCompositeWidget(parent)
{
    WContainerWidget *weatherWidgetRow = WW<WContainerWidget>().css("row");
    WContainerWidget *weatherWidget = WW<WContainerWidget>().css("container").add(weatherWidgetRow);
    OpenWeather openWeather;
    auto forecast = openWeather.forecast(latlng, cityName);
    if(!forecast) {
        setImplementation(new WText{"Weather unavailable"});
        return;
    }
    for(Weather weather: forecast->weathers()) {
        WTemplate *weatherItemWidget = new WTemplate(WString::tr("weather_item"));
        weatherItemWidget->bindString("weather-date", WDateTime::fromPosixTime(weather.dateGMT()).toString("dd/MM"));
        weatherItemWidget->bindString("weather-name", WString::fromUTF8(weather.summaries()[0].main()));
        weatherItemWidget->bindString("weather-description", WString::fromUTF8(weather.summaries()[0].description()));
        weatherItemWidget->bindString("weather-icon-url", WString::fromUTF8(weather.summaries()[0].iconURL()));

        weatherItemWidget->bindInt("clouds", weather.clouds());
        weatherItemWidget->bindInt("humidity", weather.humidity());
        weatherItemWidget->bindString("temp-min", format("%.1f") % weather.temperature().min().celsius());
        weatherItemWidget->bindString("temp-max", format("%.1f") % weather.temperature().max().celsius());
        weatherWidgetRow->addWidget(weatherItemWidget);
    }
    setImplementation(weatherWidget);
}
