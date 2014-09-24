#include "weatherforecast.h"
#include <Wt/Json/Array>
using namespace std;
using namespace Wt;

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

WeatherForecast::WeatherForecast(const Json::Object &json)
    : _city(json.get("city"))
{
    _count = json.get("cnt").toNumber().orIfNull(0);
    _cod = json.get("cod").toString().orIfNull("");
    Json::Array weathers = json.get("list");
    for(auto w: weathers) {
        _weathers.push_back({w});
    }
}

WeatherForecast::~WeatherForecast()
{
}
