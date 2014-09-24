#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H
#include <Wt/WCompositeWidget>
#include "types.h"

class WeatherWidget : public Wt::WCompositeWidget
{
public:
    WeatherWidget(const Coordinates::LatLng latlng, const std::string &cityName, const boost::posix_time::ptime &date, Wt::WContainerWidget *parent = 0);
};

#endif // WEATHERWIDGET_H
