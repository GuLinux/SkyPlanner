#include "weatherwidget.h"
#include "openweather/openweather.h"
#include <Wt-Commons/wt_helpers.h>
#include <Wt/WText>
#include <Wt/WTemplate>
#include <Wt/WDateTime>
#include <Wt/WPushButton>
#include "utils/format.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;
WeatherWidget::WeatherWidget(const Coordinates::LatLng latlng, const GeoCoder::Place &place, const boost::posix_time::ptime &date, Format widgetFormat, WContainerWidget *parent)
    : WCompositeWidget(parent), widgetFormat(widgetFormat)
{
    // TODO: translate
    weatherWidget = WW<WContainerWidget>().css("container");
    setImplementation(weatherWidget);
    reload(latlng, place, date);
}


void WeatherWidget::reload(const Coordinates::LatLng latlng, const GeoCoder::Place &place, const boost::posix_time::ptime &date)
{
    weatherWidget->clear();
    OpenWeather openWeather;
    boost::posix_time::time_duration difference(date - boost::posix_time::second_clock().local_time());
    auto forecast = openWeather.forecast(latlng, format("%s, %s") % place.city % place.country, difference.total_seconds() / 60 / 60 /24);
    if(!forecast && widgetFormat == Format::Full) {
        weatherWidget->addWidget((new WText{WString::tr("weather_not_available")}));
        weatherWidget->addWidget(WW<WPushButton>(WString::tr("button_retry")).css("btn-link").onClick([=](WMouseEvent){ reload(latlng, place, date); }));
        return;
    }
    WContainerWidget *weatherWidgetRow = WW<WContainerWidget>().css("row");
    weatherWidget->addWidget(weatherWidgetRow);
    weatherWidget->setStyleClass("container");
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
}
