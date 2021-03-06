#define BOOST_TEST_MAIN
#include "test_helper.h"
#include "openweather/weatherforecast.h"
#include <Wt/Json/Parser>
#include <Wt/Json/Object>

using namespace std;
using namespace Wt;

struct F {
    F() : json(R"({"cod":"200","message":0.5166,"city":{"id":"3179092","name":"Ceresole Reale","coord":{"lon":7.20173,"lat":45.4479},"country":"Italy","population":2345},"cnt":16,"list":[{"dt":1411556400,"temp":{"day":290.72,"min":285.27,"max":290.72,"night":285.27,"eve":288.68,"morn":290.72},"pressure":884.41,"humidity":73,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":1.16,"deg":105,"clouds":68,"rain":0.5},{"dt":1411642800,"temp":{"day":293.12,"min":282.07,"max":293.12,"night":282.24,"eve":287.97,"morn":282.07},"pressure":885.06,"humidity":61,"weather":[{"id":800,"main":"Clear","description":"sky is clear","icon":"01d"}],"speed":1.16,"deg":105,"clouds":0},{"dt":1411729200,"temp":{"day":291.07,"min":279.12,"max":291.07,"night":279.12,"eve":285.98,"morn":279.85},"pressure":890.51,"humidity":54,"weather":[{"id":800,"main":"Clear","description":"sky is clear","icon":"01d"}],"speed":1.11,"deg":103,"clouds":0},{"dt":1411815600,"temp":{"day":291.44,"min":275.51,"max":291.44,"night":277.91,"eve":287.19,"morn":275.51},"pressure":868.89,"humidity":0,"weather":[{"id":800,"main":"Clear","description":"sky is clear","icon":"01d"}],"speed":0.53,"deg":93,"clouds":0},{"dt":1411902000,"temp":{"day":291.36,"min":276.31,"max":291.36,"night":279.17,"eve":287.56,"morn":276.31},"pressure":869.62,"humidity":0,"weather":[{"id":800,"main":"Clear","description":"sky is clear","icon":"01d"}],"speed":0.59,"deg":105,"clouds":1},{"dt":1411988400,"temp":{"day":291.81,"min":276.96,"max":291.81,"night":276.96,"eve":285.95,"morn":277.49},"pressure":868.73,"humidity":0,"weather":[{"id":800,"main":"Clear","description":"sky is clear","icon":"01d"}],"speed":0.58,"deg":181,"clouds":0},{"dt":1412074800,"temp":{"day":289.05,"min":275.36,"max":289.05,"night":276.61,"eve":284.51,"morn":275.36},"pressure":863.81,"humidity":0,"weather":[{"id":501,"main":"Rain","description":"moderate rain","icon":"10d"}],"speed":0.85,"deg":229,"clouds":64,"rain":6.77},{"dt":1412161200,"temp":{"day":286,"min":276.16,"max":286,"night":279.41,"eve":281.72,"morn":276.16},"pressure":867.79,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.62,"deg":99,"clouds":40,"rain":1.92},{"dt":1412247600,"temp":{"day":283.34,"min":276.99,"max":283.34,"night":276.99,"eve":281.25,"morn":278.94},"pressure":869.6,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.33,"deg":173,"clouds":71,"rain":1.21},{"dt":1412334000,"temp":{"day":282.81,"min":276.99,"max":282.81,"night":276.99,"eve":279.87,"morn":278.07},"pressure":863.83,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.81,"deg":9,"clouds":58,"rain":2.21},{"dt":1412420400,"temp":{"day":282.94,"min":272.1,"max":282.94,"night":272.1,"eve":278.67,"morn":274.36},"pressure":857.24,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.7,"deg":8,"clouds":35,"rain":1.18},{"dt":1412506800,"temp":{"day":283.01,"min":271.4,"max":283.01,"night":276.53,"eve":278.71,"morn":271.4},"pressure":855.23,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.86,"deg":206,"clouds":22,"rain":1.51},{"dt":1412593200,"temp":{"day":282.44,"min":275.03,"max":282.44,"night":275.03,"eve":279.13,"morn":277.22},"pressure":857.25,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.86,"deg":145,"clouds":34,"rain":2.06},{"dt":1412679600,"temp":{"day":284.97,"min":274.46,"max":284.97,"night":275.61,"eve":280,"morn":274.46},"pressure":862.01,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.96,"deg":100,"clouds":1,"rain":0.94},{"dt":1412766000,"temp":{"day":284.8,"min":273.9,"max":284.8,"night":273.9,"eve":280.35,"morn":276.2},"pressure":865.68,"humidity":0,"weather":[{"id":500,"main":"Rain","description":"light rain","icon":"10d"}],"speed":0.9,"deg":128,"clouds":1},{"dt":1412852400,"temp":{"day":273.9,"min":273.9,"max":273.9,"night":273.9,"eve":273.9,"morn":273.9},"pressure":866.35,"humidity":0,"weather":[{"id":800,"main":"Clear","description":"sky is clear","icon":"01ddd"}],"speed":0.51,"deg":275,"clouds":0}]})")
    {
        Json::parse(json, weatherForecastObject);
        BOOST_REQUIRE_EQUAL("200", weatherForecastObject.get("cod").orIfNull("<NULL>"));
    }

    ~F()         {}

    const string json;
    Json::Object weatherForecastObject;
};

BOOST_FIXTURE_TEST_CASE(TestParseWeatherForecast, F) {
    WeatherForecast forecast(weatherForecastObject);
    BOOST_REQUIRE_EQUAL("200", forecast.cod());
    BOOST_REQUIRE_EQUAL(16, forecast.count());
}
BOOST_FIXTURE_TEST_CASE(TestWeatherCityParsing, F) {
    WeatherForecast forecast(weatherForecastObject);
    BOOST_REQUIRE_EQUAL(3179092, forecast.city().id());
    BOOST_REQUIRE_EQUAL("Ceresole Reale", forecast.city().name());
    BOOST_REQUIRE_EQUAL(45.4479, forecast.city().coordinates().latitude.degrees());
    BOOST_REQUIRE_EQUAL(7.20173, forecast.city().coordinates().longitude.degrees());
    BOOST_REQUIRE_EQUAL("Italy", forecast.city().country());
    BOOST_REQUIRE_EQUAL(2345, forecast.city().population());
}
BOOST_FIXTURE_TEST_CASE(TestWeatherParsing, F) {
    WeatherForecast forecast(weatherForecastObject);
    BOOST_REQUIRE_EQUAL(16, forecast.weathers().size());
    auto date = boost::posix_time::from_iso_string("20140924T110000");
    BOOST_REQUIRE_EQUAL(date, forecast.weathers()[0].dateGMT());
    auto temperature = forecast.weathers()[1].temperature();
    BOOST_REQUIRE_CLOSE(293.12f, temperature.day().kelvin, 0.01);
    BOOST_REQUIRE_CLOSE(8.92f, temperature.min().celsius(), 0.01);
}


/* Formatted json test string:
{
   "cod":"200",
   "message":0.5166,
   "city":{
      "id":"3179092",
      "name":"Ceresole Reale",
      "coord":{
         "lon":7.20173,
         "lat":45.4479
      },
      "country":"Italy",
      "population":0
   },
   "cnt":16,
   "list":[
      {
         "dt":1411556400,
         "temp":{
            "day":290.72,
            "min":285.27,
            "max":290.72,
            "night":285.27,
            "eve":288.68,
            "morn":290.72
         },
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
      {
         "dt":1411642800,
         "temp":{
            "day":293.12,
            "min":282.07,
            "max":293.12,
            "night":282.24,
            "eve":287.97,
            "morn":282.07
         },
         "pressure":885.06,
         "humidity":61,
         "weather":[
            {
               "id":800,
               "main":"Clear",
               "description":"sky is clear",
               "icon":"01d"
            }
         ],
         "speed":1.16,
         "deg":105,
         "clouds":0
      },
      {
         "dt":1411729200,
         "temp":{
            "day":291.07,
            "min":279.12,
            "max":291.07,
            "night":279.12,
            "eve":285.98,
            "morn":279.85
         },
         "pressure":890.51,
         "humidity":54,
         "weather":[
            {
               "id":800,
               "main":"Clear",
               "description":"sky is clear",
               "icon":"01d"
            }
         ],
         "speed":1.11,
         "deg":103,
         "clouds":0
      },
      {
         "dt":1411815600,
         "temp":{
            "day":291.44,
            "min":275.51,
            "max":291.44,
            "night":277.91,
            "eve":287.19,
            "morn":275.51
         },
         "pressure":868.89,
         "humidity":0,
         "weather":[
            {
               "id":800,
               "main":"Clear",
               "description":"sky is clear",
               "icon":"01d"
            }
         ],
         "speed":0.53,
         "deg":93,
         "clouds":0
      },
      {
         "dt":1411902000,
         "temp":{
            "day":291.36,
            "min":276.31,
            "max":291.36,
            "night":279.17,
            "eve":287.56,
            "morn":276.31
         },
         "pressure":869.62,
         "humidity":0,
         "weather":[
            {
               "id":800,
               "main":"Clear",
               "description":"sky is clear",
               "icon":"01d"
            }
         ],
         "speed":0.59,
         "deg":105,
         "clouds":1
      },
      {
         "dt":1411988400,
         "temp":{
            "day":291.81,
            "min":276.96,
            "max":291.81,
            "night":276.96,
            "eve":285.95,
            "morn":277.49
         },
         "pressure":868.73,
         "humidity":0,
         "weather":[
            {
               "id":800,
               "main":"Clear",
               "description":"sky is clear",
               "icon":"01d"
            }
         ],
         "speed":0.58,
         "deg":181,
         "clouds":0
      },
      {
         "dt":1412074800,
         "temp":{
            "day":289.05,
            "min":275.36,
            "max":289.05,
            "night":276.61,
            "eve":284.51,
            "morn":275.36
         },
         "pressure":863.81,
         "humidity":0,
         "weather":[
            {
               "id":501,
               "main":"Rain",
               "description":"moderate rain",
               "icon":"10d"
            }
         ],
         "speed":0.85,
         "deg":229,
         "clouds":64,
         "rain":6.77
      },
      {
         "dt":1412161200,
         "temp":{
            "day":286,
            "min":276.16,
            "max":286,
            "night":279.41,
            "eve":281.72,
            "morn":276.16
         },
         "pressure":867.79,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.62,
         "deg":99,
         "clouds":40,
         "rain":1.92
      },
      {
         "dt":1412247600,
         "temp":{
            "day":283.34,
            "min":276.99,
            "max":283.34,
            "night":276.99,
            "eve":281.25,
            "morn":278.94
         },
         "pressure":869.6,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.33,
         "deg":173,
         "clouds":71,
         "rain":1.21
      },
      {
         "dt":1412334000,
         "temp":{
            "day":282.81,
            "min":276.99,
            "max":282.81,
            "night":276.99,
            "eve":279.87,
            "morn":278.07
         },
         "pressure":863.83,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.81,
         "deg":9,
         "clouds":58,
         "rain":2.21
      },
      {
         "dt":1412420400,
         "temp":{
            "day":282.94,
            "min":272.1,
            "max":282.94,
            "night":272.1,
            "eve":278.67,
            "morn":274.36
         },
         "pressure":857.24,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.7,
         "deg":8,
         "clouds":35,
         "rain":1.18
      },
      {
         "dt":1412506800,
         "temp":{
            "day":283.01,
            "min":271.4,
            "max":283.01,
            "night":276.53,
            "eve":278.71,
            "morn":271.4
         },
         "pressure":855.23,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.86,
         "deg":206,
         "clouds":22,
         "rain":1.51
      },
      {
         "dt":1412593200,
         "temp":{
            "day":282.44,
            "min":275.03,
            "max":282.44,
            "night":275.03,
            "eve":279.13,
            "morn":277.22
         },
         "pressure":857.25,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.86,
         "deg":145,
         "clouds":34,
         "rain":2.06
      },
      {
         "dt":1412679600,
         "temp":{
            "day":284.97,
            "min":274.46,
            "max":284.97,
            "night":275.61,
            "eve":280,
            "morn":274.46
         },
         "pressure":862.01,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.96,
         "deg":100,
         "clouds":1,
         "rain":0.94
      },
      {
         "dt":1412766000,
         "temp":{
            "day":284.8,
            "min":273.9,
            "max":284.8,
            "night":273.9,
            "eve":280.35,
            "morn":276.2
         },
         "pressure":865.68,
         "humidity":0,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":0.9,
         "deg":128,
         "clouds":1
      },
      {
         "dt":1412852400,
         "temp":{
            "day":273.9,
            "min":273.9,
            "max":273.9,
            "night":273.9,
            "eve":273.9,
            "morn":273.9
         },
         "pressure":866.35,
         "humidity":0,
         "weather":[
            {
               "id":800,
               "main":"Clear",
               "description":"sky is clear",
               "icon":"01ddd"
            }
         ],
         "speed":0.51,
         "deg":275,
         "clouds":0
      }
   ]
}
*/
