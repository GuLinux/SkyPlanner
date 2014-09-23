#ifndef WEATHERCITY_H
#define WEATHERCITY_H
#include "utils/d_ptr.h"
#include "types.h"
class WeatherCity
{
public:
    WeatherCity();
    ~WeatherCity();
    uint64_t id() const;
    std::string name() const;
    Coordinates::LatLng coordinates() const;
    std::string country() const;
    uint64_t population() const;
private:
    D_PTR;
};

#endif // WEATHERCITY_H
