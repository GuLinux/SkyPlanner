#ifndef WEATHER_P_H
#define WEATHER_P_H
#include "openweather/weather.h"

class Weather::Private {
public:
    uint64_t dataReceivingTime;
    uint64_t kelvinTempDay;
    uint64_t kelvinTempMin;
    uint64_t kelvinTempMax;
    uint64_t kelvinTempNight;
    uint64_t kelvinTempEve;
    uint64_t kelvinTempMorn;
    uint64_t pressure;
    uint64_t humidity;
};

#endif // WEATHER_P_H
