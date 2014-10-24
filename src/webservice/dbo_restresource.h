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
  class not_found_exception : public std::runtime_error {
    not_found_exception() : std::runtime_error("404/Not Found") {}
  };
};

template<typename T> void DboRestsResource<T>::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) {
  Session session;
  Wt::Dbo::Transaction t(session);
  auto query = session.find<T>();
  for(auto handler: handlers) {
    if(handler.matches(query, request)) {
      if(handler.type == Find::Single) {
	auto result = query.resultValue();
	if(!result) {
	  response.setStatus(404);
	  return;
	}
	response.setStatus(200);
	response.out() << result->toJson();
	return;
      }
      response.setStatus(200);
      auto objects = query.resultList();
      WtCommons::Json::Array<Wt::Dbo::ptr<T>, Wt::Dbo::collection, WtCommons::Json::PointerObjectConverter<T, Wt::Dbo::ptr>> jsonArray(objects);
      response.out() << jsonArray.toJson();
      return;
    }
  }
  response.setStatus(404);
}




#endif // SKYOBJECTSRESOURCE_H


