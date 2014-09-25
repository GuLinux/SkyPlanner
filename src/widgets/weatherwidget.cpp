#include "weatherwidget.h"
#include "openweather/openweather.h"
#include <Wt-Commons/wt_helpers.h>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WDateTime>
#include <Wt/WPushButton>
#include "utils/format.h"
#include "skyplanner.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
WeatherWidget::WeatherWidget(const Coordinates::LatLng latlng, const GeoCoder::Place &place, const boost::posix_time::ptime &date, Format widgetFormat, WContainerWidget *parent)
    : WCompositeWidget(parent), widgetFormat(widgetFormat)
{
    weatherWidget = WW<WContainerWidget>().css("container");
    setImplementation(weatherWidget);
    reload(latlng, place, date);
}


void WeatherWidget::reload(const Coordinates::LatLng latlng, const GeoCoder::Place &place, const boost::posix_time::ptime &date)
{
    weatherWidget->clear();
    weatherWidget->setStyleClass("container-fluid");
    OpenWeather openWeather;
    boost::posix_time::time_duration difference(date - boost::posix_time::second_clock().local_time());
    auto days = (difference.total_seconds() / 60 / 60 / 24 ) + 1;
    days = std::max(days, 4);
    spLog("notice") << "Getting forecast for the time_difference: " << boost::posix_time::to_simple_string(difference) << ", secs=" << difference.total_seconds();
    auto forecast = openWeather.forecast(latlng, format("%s, %s") % place.city % place.country, days);
    if(!forecast && widgetFormat == Format::Full) {
        weatherWidget->addWidget((new WText{WString::tr("weather_not_available")}));
        weatherWidget->addWidget(WW<WPushButton>(WString::tr("button_retry")).css("btn-link").onClick([=](WMouseEvent){ reload(latlng, place, date); }));
        return;
    }
    if(widgetFormat == Full) {
        weatherWidget->addWidget(WW<WContainerWidget>().css("row")
          .add(WW<WText>(WString::tr("weather_city_title").arg(forecast->city().name()).arg(forecast->city().country())).css("col-xs-offset-3 col-xs-9")));
    }
    WContainerWidget *weatherWidgetRow = WW<WContainerWidget>().css("row");
    weatherWidget->addWidget(weatherWidgetRow);
    for(Weather weather: forecast->weathers()) {
        WTemplate *weatherItemWidget = new WTemplate(WString::tr("weather_item"));
        weatherItemWidget->addFunction("tr", &WTemplate::Functions::tr);
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
    weatherWidgetRow->addWidget(WW<WContainerWidget>().css("row").add(WW<WText>(WString::tr("openweather_credits")).css("col-xs-12 text-right")));
}
