#include "geocoder.h"
#include "utils/format.h"
#include "utils/curl.h"
#include "skyplanner.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/Json/Array>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Value>
#include "skyplanner.h"
#include "utils/curl.h"

using namespace Wt;
using namespace std;

class GeoCoder::Private {
public:
  Private(const string &apiKey, GeoCoder *q) : apiKey(apiKey), q(q) {}
  string apiKey;
private:
  GeoCoder *const q;
};

GeoCoder::GeoCoder(const string &apiKey) : d(apiKey, this)
{
}

GeoCoder::~GeoCoder()
{
}

GeoCoder::Place::operator bool() const
{
  if(city.empty() || country.empty()) return false;
  return true;
}

GeoCoder::Place GeoCoder::reverse(const Coordinates::LatLng &latlng) const
{
  stringstream out;
  string url = format("https://maps.googleapis.com/maps/api/geocode/json?latlng=%f,%f&sensor=false&key=%s&language=%s")
  % latlng.latitude.degrees()
  % latlng.longitude.degrees()
  % d->apiKey
  % wApp->locale().name();
  Curl curl(out);
  if(!curl.get(url).requestOk())
    return {};
  Json::Object o;
  Json::parse(out.str(), o);
  if(o.get("status").orIfNull("") != "OK")
    return {};
  
  Json::Array results = o.get("results");
  Place place;
  for(Json::Object result: results) {
    place.formattedAddress = result.get("formatted_address").orIfNull("");
    Json::Array addressComponents = result.get("address_components");
    for(Json::Object component: addressComponents) {
      Json::Array componentTypes = component.get("types");
      set<string> sComponentTypes;
      transform(begin(componentTypes), end(componentTypes), inserter(sComponentTypes, begin(sComponentTypes)), [](const Json::Value &v) { return v.orIfNull(""); } );
      if(sComponentTypes.count("street_address") || sComponentTypes.count("route") || sComponentTypes.count("intersection") )
	place.address = component.get("long_name").orIfNull("");
      if(sComponentTypes.count("administrative_area_level_3") || sComponentTypes.count("locality") )
	place.city = component.get("long_name").orIfNull("");
      if(sComponentTypes.count("administrative_area_level_2") ) {
	place.province = component.get("long_name").orIfNull("");
	place.provinceCode = component.get("short_name").orIfNull("");
      }
      if(sComponentTypes.count("administrative_area_level_1"))
	place.region = component.get("long_name").orIfNull("");
      if(sComponentTypes.count("country"))
	place.country = component.get("long_name").orIfNull("");
      if(sComponentTypes.count("postal_code"))
	place.zipCode = component.get("long_name").orIfNull("");
    }
    return place;
  }
  return {};
}


GeoCoder::PlaceInformation GeoCoder::placeInformation(const Coordinates::LatLng &coordinates, const boost::posix_time::ptime &when)
{
  static string googleApiKey;
  if(googleApiKey.empty())
    wApp->readConfigurationProperty("google_api_server_key", googleApiKey);
  PlaceInformation placeInformation;
  static map<string,Timezone> timezonesCache;
  if(coordinates){
    string key = Timezone::key(coordinates.latitude.degrees(), coordinates.longitude.degrees(), when, wApp->locale().name());
    spLog("notice") << "Timezone identifier: " << key;
    if(timezonesCache.count(key)) {
      placeInformation.timezone = timezonesCache[key];
      spLog("notice") << "Timezone " << placeInformation.timezone << " found in cache, skipping webservice request";
    } else {
      tm td_tm = to_tm(when);
      time_t tt = mktime(&td_tm);
      string url = format("https://maps.googleapis.com/maps/api/timezone/json?location=%f,%f&timestamp=%d&sensor=false&key=%s&language=%s")
        % coordinates.latitude.degrees()
        % coordinates.longitude.degrees()
        % tt
        % googleApiKey
        % wApp->locale().name();
      ;
      spLog("notice") << "URL: " << url;
      stringstream data;
      Curl curl(data);
      bool getRequest = ! googleApiKey.empty() && curl.get(url).requestOk();
      GeoCoder geocoder(googleApiKey);
      placeInformation.geocoderPlace = geocoder.reverse(coordinates);
      spLog("notice") << "reverse geocoder lookup: " << placeInformation.geocoderPlace;

      spLog("notice") << "get request: " << boolalpha << getRequest << ", http code: " << curl.httpResponseCode() << ", out: " << data.str();
      if(getRequest) {
        try {
          placeInformation.timezone = Timezone::from(data.str(), coordinates.latitude.degrees(), coordinates.longitude.degrees());
          timezonesCache[key] = placeInformation.timezone;
          spLog("notice") << "got timezone info: " << placeInformation.timezone;
        } catch(std::exception &e) {
          spLog("notice") << "Unable to parse json response into a timezone object: " << e.what();
        }
      }
    }
  }
  return placeInformation;
}

ostream &operator<<(ostream &o, const GeoCoder::Place & p)
{
  o << "address=" << p.address << "city=" << p.city << ", province=" << p.province << " (" << p.provinceCode << "), zipCode=" << p.zipCode << ", region=" << p.region << ", country=" << p.country << "; formatted address: " << p.formattedAddress;
  return o;
}