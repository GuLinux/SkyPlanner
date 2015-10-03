/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
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
#ifndef SKYOBJECTSRESOURCE_H
#define SKYOBJECTSRESOURCE_H

#include <Wt/WResource>
#include "session.h"
#include "Models"
#include <Wt/Http/Response>
#include <Wt/Json/Object>
#include <Wt/Json/Array>
#include <Wt/Json/Value>
#include <Wt/Json/Serializer>
#include <boost/regex.hpp>
#include <list>
#include <Wt/Dbo/Query>

class SkyPlanner;
template<typename T>
class DboRestsResource : public Wt::WResource {
  typedef Wt::Dbo::ptr<T> type_ptr;
  typedef Wt::Dbo::Query<type_ptr> type_query;
public:
    struct Find {
      typedef std::function<void(type_query &, const Wt::Http::Request &, const boost::smatch &)> Finder;
      boost::regex pathinfo_regex;
      Finder filterQuery;
      Find() = default;
      enum Type {Single, Collection} type = Collection;
      Find(const boost::regex &regex, Finder finder, Type type = Collection)
	: pathinfo_regex(regex), filterQuery(finder), type(type) {}

      bool matches(type_query &query, const Wt::Http::Request &request) const {
	boost::smatch match;
	if(!boost::regex_match(request.pathInfo(), match, pathinfo_regex))
	  return false;
	filterQuery(query, request, match);
	return true;
      }

    };
    explicit DboRestsResource(Wt::WObject *parent = 0) {}
    ~DboRestsResource() {}
    virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
    DboRestsResource<T> *handle(const Find &handler) { handlers.push_back(handler); return this; }
    DboRestsResource<T> *handleById() { return handle(Find(boost::regex("^/(\\d+)$"), [](type_query &q, const Wt::Http::Request &r, const boost::smatch &s){
      q.where("id = ?").bind(boost::lexical_cast<long long>(s[1]));
    }, Find::Single )); }
    DboRestsResource<T> *handleAll() { return handle(Find(boost::regex("^/*$"), [](type_query &q, const Wt::Http::Request &r, const boost::smatch &s){} )); }
private:
  std::list<Find> handlers;
};

template<typename T> Wt::Json::Object obj2json(const Wt::Dbo::ptr<T> &p) {
  Wt::Json::Object o = *p;
  o["id"] = p.id();
  return o;
}


template<typename T> void DboRestsResource<T>::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) {
  Session session;
  Wt::Dbo::Transaction t(session);
  auto query = session.find<T>();
  for(auto handler: handlers) {
    if(handler.matches(query, request)) {
      std::cerr << "query matches handler: " << handler.pathinfo_regex << std::endl;
      if(handler.type == Find::Single) {
	auto result = query.resultValue();
	if(!result) {
	  response.setStatus(404);
	  return;
	}
	response.setStatus(200);
	response.out() << Wt::Json::serialize(obj2json<T>(result));
	return;
      }
      response.setStatus(200);
      auto objects = query.resultList();
      Wt::Json::Array json_array;
      std::transform(std::begin(objects), std::end(objects), std::back_inserter(json_array), [=](const Wt::Dbo::ptr<T> &obj){
	Wt::Json::Value v(Wt::Json::ObjectType);
	Wt::Json::Object &o = v;
	o = obj2json<T>(obj);
	return v;
      });
      response.out() << Wt::Json::serialize(json_array);
      return;
    }
  }
  response.setStatus(404);
}




#endif // SKYOBJECTSRESOURCE_H


