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

#include "ngcresource.h"
#include "session.h"
#include "ngcobject.h"
#include "nebuladenomination.h"
#include <Wt/Http/Response>
#include <Wt/Json/Object>
#include <Wt/Json/Serializer>
#include <Wt/Json/Array>
#include <map>
using namespace std;
using namespace Wt;

NgcResource::NgcResource(WObject* parent)
  : WResource(parent)
{

}
NgcResource::~NgcResource()
{

}

map<string,string> Catalogs {
  {"C", "Caldwell"},
  {"M", "Messier"},
  {"NGC", "NGC"},
  {"IC", "IC"},
};

void NgcResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
  Session session;
  Json::Object ngcObjectJson;
  ngcObjectJson["result"] = Json::Value("ok");
  
  if(request.getParameter("cat") == 0 || request.getParameter("num") == 0) {
    ngcObjectJson["result"] = Json::Value(string("error"));
    ngcObjectJson["error_description"] = Json::Value(request.getParameter("num") == 0 ? string("catalogue number missing") : string("catalogue missing"));
    response.out() << Json::serialize(ngcObjectJson) << endl;
    return;
  }
  string catalogue = Catalogs[*request.getParameter("cat")];
  string number = *request.getParameter("num");
  Dbo::Transaction t(session);
  Dbo::ptr<NebulaDenomination> objectDenomination = session.find<NebulaDenomination>().where("catalogue = ?").where("number = ?").bind(catalogue).bind(number);
  if(!objectDenomination) {
    ngcObjectJson["result"] = Json::Value(string("error"));
    stringstream errorDescription ;
    errorDescription << "Object " << catalogue << " " << number << " not found!";
    ngcObjectJson["error_description"] = Json::Value(errorDescription.str());
    response.out() << Json::serialize(ngcObjectJson) << endl;
    response.out()  << endl;
    return;
  }
  Dbo::ptr<NgcObject> object = objectDenomination->ngcObject();
  ngcObjectJson["ar"] = Json::Value(object->rightAscension());
  ngcObjectJson["dec"] = Json::Value(object->declination());
  ngcObjectJson["magnitude"] = Json::Value(object->magnitude());
  ngcObjectJson["angular_size"] = Json::Value(object->angularSize());
  ngcObjectJson["type"] = Json::Value(object->type());
  ngcObjectJson["type_description"] = Json::Value(object->typeDescription());
  Json::Value denominations(Json::ArrayType);
  for(Dbo::ptr<NebulaDenomination> denomination: object->nebulae()) {
    Json::Value denominationJson(Json::ObjectType);
    static_cast<Json::Object&>(denominationJson)["catalogue"] = Json::Value(denomination->catalogue());
    static_cast<Json::Object&>(denominationJson)["number"] = Json::Value(denomination->number());
    static_cast<Json::Object&>(denominationJson)["name"] = Json::Value(denomination->name());
    static_cast<Json::Object&>(denominationJson)["comment"] = Json::Value(denomination->comment());
    static_cast<Json::Array&>(denominations).push_back(denominationJson);
  }
  ngcObjectJson["denominations"] = denominations;
  response.out() << Json::serialize(ngcObjectJson) << endl;
}
