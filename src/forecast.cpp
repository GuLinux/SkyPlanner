/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "forecast.h"
#include "private/forecast_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/format.h"
#include <Wt/Http/Client>
#include <boost/format.hpp>
#include <Wt/WApplication>
#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include <Wt/Json/Array>
#include "Wt-Commons/wt_helpers.h"

using namespace std;
using namespace Wt;

Forecast::Private::Private(Forecast* q) : q(q)
{
  client.setTimeout(15);
  app = wApp;
  client.done().connect([=](boost::system::error_code e, Http::Message m, _n4){
    if(e.value() != boost::system::errc::success) {
      WServer::instance()->log("error") << "Error fetching http message: " << e.value() << ", " << e.message();
      return;
    }
    if(m.status() == 302) {
      const string *newLocation = m.getHeader("Location");
      if(!newLocation) {
	WServer::instance()->log("error") << "error: got redirect with no location!";
	for(auto header: m.headers())
	  WServer::instance()->log("error") << "header: " << header.name() << ": " << header.value();
	return;
      }
      WServer::instance()->log("error") << "got redirect, trying with: " << *newLocation;
      client.get(*newLocation);
      return;
    }
    if(m.status() == 200) {
      parseForecast(m.body());
      return;
    }
  });
}

Forecast::Forecast()
    : d(this)
{
}

Forecast::~Forecast()
{
}
void Forecast::fetch(double latitude, double longitude)
{
  d->client.get(format("http://www.7timer.com/v4/bin/api.pl?lon=%f&lat=%f&product=astro&output=json") % longitude % latitude );
}
namespace {
template<typename T>
T get(Json::Object &o, string key) {
  Json::Value v = o[key];
  return v;
}
}
void Forecast::Private::parseForecast(const string &forecast)
{
  Json::Object forecastObject;
  Json::parse(forecast, forecastObject);
  const WDateTime init = WDateTime::fromString(get<string>(forecastObject, "init"), "yyyyMMdd");
  Json::Array dataseries = forecastObject["dataseries"];
  static map<string, Data::PrecipitationType> precipitations {
    {"frzr", Data::PrecipitationType::frzr},
    {"icep", Data::PrecipitationType::icep},
    {"none", Data::PrecipitationType::none},
    {"rain", Data::PrecipitationType::rain},
    {"snow", Data::PrecipitationType::snow},
  };
  vector<Forecast::Data> dataSeries;
  for(Json::Value &d: dataseries) {
    Forecast::Data data;
    int timepoint = get<int>(d, "timepoint");
    data.when = init.addSecs(timepoint * 60 * 60);
    data.cloudcover = get<int>(d, "cloudcover");
    data.seeing = get<int>(d, "seeing");
    data.transparency = get<int>(d, "transparency");
    data.precipitations = precipitations[get<string>(d, "prec_type")];
    dataSeries.push_back(data);
  }
  for(auto d: dataSeries) {
    cerr << "forecast for " << d.when.toString() << ": cloudcover=" << d.cloudcover << ", seeing=" << d.seeing << ", transparency: " << d.transparency << endl;
  }
}
