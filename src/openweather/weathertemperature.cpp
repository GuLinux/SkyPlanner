#include "weathertemperature.h"
using namespace Wt;
using namespace std;
WeatherTemperature::WeatherTemperature(const Json::Object &json)
         : _day{json.get("day").toNumber().orIfNull(0.f)},
           _min{json.get("min").toNumber().orIfNull(0.f)},
           _max{json.get("max").toNumber().orIfNull(0.f)},
           _night{json.get("night").toNumber().orIfNull(0.f)},
           _eve{json.get("eve").toNumber().orIfNull(0.f)},
           _morn{json.get("morn").toNumber().orIfNull(0.f)}
{
}

/*
    Temperature _day;
    Temperature _min;
    Temperature _max;
    Temperature _night;
    Temperature _eve;
    Temperature _morn;
    */
