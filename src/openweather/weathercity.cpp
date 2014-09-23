#include "weathercity.h"
#include "private/weathercity_p.h"
#include "utils/d_ptr_implementation.h"

/*
   "city":{
      "id":1851632,
      "name":"Shuzenji",
      "coord":{
         "lon":138.933334,
         "lat":34.966671
      },
      "country":"JP",
      "population":0
   },
*/


WeatherCity::WeatherCity()
{
}

WeatherCity::~WeatherCity()
{
}

uint64_t WeatherCity::id() const
{
    return d->id;
}

std::string WeatherCity::name() const
{
    return d->name;
}

Coordinates::LatLng WeatherCity::coordinates() const
{
    return d->coordinates;
}

std::string WeatherCity::country() const
{
    return d->country;
}

uint64_t WeatherCity::population() const
{
    return d->population;
}
