#ifndef WEATHERTEMPERATURE_H
#define WEATHERTEMPERATURE_H
#include <Wt/Json/Object>

class WeatherTemperature
{
public:
    static constexpr float K2C = -273.15;
    struct Temperature {
        float kelvin;
        float celsius() const { return K2C + kelvin; }
    };

    WeatherTemperature(const Wt::Json::Object &json);
    Temperature day() const { return _day; }
    Temperature min() const { return _min; }
    Temperature max() const { return _max; }
    Temperature night() const { return _night; }
    Temperature eve() const { return _eve; }
    Temperature morn() const { return _morn; }

private:
    Temperature _day;
    Temperature _min;
    Temperature _max;
    Temperature _night;
    Temperature _eve;
    Temperature _morn;
};

#endif // WEATHERTEMPERATURE_H
