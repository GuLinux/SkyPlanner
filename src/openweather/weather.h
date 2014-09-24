#ifndef WEATHER_H
#define WEATHER_H

#include <boost/date_time.hpp>
#include "types.h"
#include <Wt/Json/Object>

#include "weathertemperature.h"
#include "weathersummary.h"

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

class Weather
{
public:
    Weather(const Wt::Json::Object &json);
    ~Weather();
    boost::posix_time::ptime dateGMT() const { return _dateGMT; }
    WeatherTemperature temperature() const { return _temperature; }
    std::vector<WeatherSummary> summaries() const { return _summaries; }
    float pressure() const { return _pressure; }
    int humidity() const { return _humidity; }
    float windSpeed() const { return _windSpeed; }
    Angle windAngle() const { return _windAngle; }
    float rain() { return _rain; }
    int clouds() { return _clouds; }
private:
    boost::posix_time::ptime _dateGMT;
    WeatherTemperature _temperature;
    float _pressure;
    int _humidity;
    float _windSpeed;
    Angle _windAngle;
    float _rain;
    int _clouds;
    std::vector<WeatherSummary> _summaries;
};

#endif // WEATHER_H
