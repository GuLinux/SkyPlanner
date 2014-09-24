#include "weather.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/Json/Array>

/*
{
         "dt":1411556400,
         "temp": {...},
         "pressure":884.41,
         "humidity":73,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":1.16,
         "deg":105,
         "clouds":68,
         "rain":0.5
      },
*/
using namespace Wt;
using namespace std;

Weather::Weather(const Json::Object &json)
    : _temperature(json.get("temp"))
{
    _dateGMT = boost::posix_time::from_time_t(json.get("dt").orIfNull(0));
    _pressure = json.get("pressure").orIfNull(0.d);
    _humidity = json.get("humidity").orIfNull(0);
    _windSpeed = json.get("speed").orIfNull(0.);
    _windAngle = Angle::degrees(json.get("deg").orIfNull(0));
    _clouds = json.get("clouds").orIfNull(0);
    _rain = json.get("clouds").orIfNull(0.);
    Json::Array weathers = json.get("weather");
    for(auto w: weathers) {
        _summaries.push_back({w});
    }
}

Weather::~Weather()
{
}
