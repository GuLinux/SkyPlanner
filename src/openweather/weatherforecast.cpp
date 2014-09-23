#include "weatherforecast.h"
#include "private/weatherforecast_p.h"
#include "utils/d_ptr_implementation.h"
using namespace std;

/*
{
   "cod":"200",
   "message":0.0167,
   "city":{... },
      "country":"JP",
      "population":0
   },
   "cnt":34,
   "list":[  ]
}
*/

WeatherForecast::WeatherForecast()
{
}

WeatherForecast::~WeatherForecast()
{
}

shared_ptr<WeatherCity> WeatherForecast::city() const {
    return d->city;
}

string WeatherForecast::cod() const
{
    return d->cod;
}
int WeatherForecast::count() const
{
    return d->count;
}
vector<std::shared_ptr<Weather>> WeatherForecast::weathers() const
{
    return d->weathers;
}
