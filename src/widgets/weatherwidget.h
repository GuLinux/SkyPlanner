#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H
#include <Wt/WCompositeWidget>
#include "types.h"
#include "geocoder.h"

class WeatherWidget : public Wt::WCompositeWidget
{
public:
    enum Format{Embedded, Full };
    WeatherWidget(const Coordinates::LatLng latlng, const GeoCoder::Place &place,
                  const boost::posix_time::ptime &date, Format widgetFormat = Embedded, Wt::WContainerWidget *parent = 0);
};

#endif // WEATHERWIDGET_H
