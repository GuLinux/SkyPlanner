#ifndef WEATHERSUMMARY_H
#define WEATHERSUMMARY_H
#include <string>
#include <Wt/Json/Object>

/*
{
   "id":500,
   "main":"Rain",
   "description":"light rain",
   "icon":"10d"
}
*/

class WeatherSummary
{
public:
    WeatherSummary(const Wt::Json::Object &object);
    std::string main() { return _main; }
    std::string description() const { return _description; }
    std::string icon() const { return _icon; }
    std::string iconURL() const;
private:
    int _id;
    std::string _main;
    std::string _description;
    std::string _icon;

};

#endif // WEATHERSUMMARY_H
