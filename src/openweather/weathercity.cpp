#include "weathercity.h"

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

using namespace std;
using namespace Wt;

WeatherCity::WeatherCity(const Json::Object &json)
{
    _id = json.get("id").toNumber().orIfNull(0);
    _name = json.get("name").toString().orIfNull("");
    Json::Object latlng = json.get("coord");
    _coordinates.latitude = Angle::degrees(latlng.get("lat").toNumber().orIfNull(0.0));
    _coordinates.longitude = Angle::degrees(latlng.get("lon").toNumber().orIfNull(0.0));
    _country = json.get("country").toString().orIfNull("");
    _population = json.get("population").toNumber().orIfNull(0);
}

WeatherCity::~WeatherCity()
{
}
