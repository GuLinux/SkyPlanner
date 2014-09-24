#include "weathersummary.h"
#include "utils/format.h"

using namespace Wt;
using namespace std;
/*
{
   "id":500,
   "main":"Rain",
   "description":"light rain",
   "icon":"10d"
}
*/


WeatherSummary::WeatherSummary(const Json::Object &object)
{
    _id = object.get("id").toNumber().orIfNull(0);
    _main = object.get("main").toString().orIfNull("");
    _description = object.get("description").toString().orIfNull("");
    _icon = object.get("icon").toString().orIfNull("");
}

string WeatherSummary::iconURL() const {
    return format("http://openweathermap.org/img/w/%s.png") % icon();
}
