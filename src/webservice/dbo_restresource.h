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
public:
    struct Find {
      boost::regex pathinfo_regex;
      typedef std::function<void(Wt::Dbo::Query<T> &, const Wt::Http::Request &, const boost::smatch &)> Finder;
      Finder filterQuery;

      bool matches(Wt::Dbo::Query<T> &query, const Wt::Http::Request &request) const {
	boost::smatch match;
	if(!boost::regex_match(request.pathInfo(), match, pathinfo_regex))
	  return false;
	filterQuery(query, request, match);
	return true;
      }
      enum Type {Single, Collection} type = Collection;

    };
    explicit DboRestsResource(Wt::WObject *parent = 0) {}
    ~DboRestsResource() {}
    virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);
    DboRestsResource<T> *handle(const Find &handler) { handlers.push_back(handler); return this; }
    DboRestsResource<T> *handleById() { return handle({"^/(\\d+)$", [](Wt::Dbo::Query<T> &q, const Wt::Http::Request &r, const boost::smatch &s){
      q.where("id = ?").bind(boost::lexical_cast<long long>(s[1]));
    } }); }
    DboRestsResource<T> *handleAll() { return handle({"^/*$", [](Wt::Dbo::Query<T> &q, const Wt::Http::Request &r, const boost::smatch &s){} }); }
private:
  std::list<Find> handlers;
  class not_found_exception : public std::runtime_error {
    not_found_exception() : std::runtime_error("404/Not Found") {}
  };
};

template<typename T> void DboRestsResource<T>::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response) {
  Session session;
  Wt::Dbo::Transaction t(session);
  boost::regex match_path_empty("^/*$");
  boost::regex get_id("^/(\\d+)$");
  boost::smatch pathinfo_matches;
  response.out() << request.pathInfo() << std::endl;
  
  if(boost::regex_match(request.pathInfo(), pathinfo_matches, match_path_empty)) {
    auto objects = session.find<NgcObject>().resultList();
    WtCommons::Json::Array<NgcObjectPtr, Wt::Dbo::collection, WtCommons::Json::PointerObjectConverter<NgcObject, Wt::Dbo::ptr>> jsonArray(objects);
    response.out() << jsonArray.toJson();
    response.setStatus(200);
  }
  
  if(boost::regex_match(request.pathInfo(), pathinfo_matches, get_id)) {
    auto object = session.find<NgcObject>().where("id = ?").bind(boost::lexical_cast<long long>(pathinfo_matches[1])).resultValue();
    if(!object) {
      response.setStatus(404);
      return;
    }
    response.out() << object->toJson();
    response.setStatus(200);
    return;
  }
  response.setStatus(404);
}




#endif // SKYOBJECTSRESOURCE_H


