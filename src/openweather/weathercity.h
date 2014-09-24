#ifndef WEATHERCITY_H
#define WEATHERCITY_H
#include "types.h"
#include <Wt/Json/Object>

class WeatherCity
{
public:
    WeatherCity(const Wt::Json::Object &json);
    ~WeatherCity();
    uint64_t id() const { return _id; }
    std::string name() const { return _name; }
    Coordinates::LatLng coordinates() const { return _coordinates; }
    std::string country() const { return _country; }
    uint64_t population() const { return _population; }
private:
    u_int64_t _id;
    std::string _name;
    Coordinates::LatLng _coordinates;
    std::string _country;
    uint64_t _population;
};

#endif // WEATHERCITY_H
