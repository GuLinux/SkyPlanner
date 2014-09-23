#ifndef WEATHERCITY_P_H
#define WEATHERCITY_P_H
#include "openweather/weathercity.h"
class WeatherCity::Private {
public:
    uint64_t id;
    std::string name;
    Coordinates::LatLng coordinates;
    std::string country;
    uint64_t population;
};

#endif // WEATHERCITY_P_H
