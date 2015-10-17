/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "findneighbour.h"
#include "c++/stlutils.h"
#include <Wt/WApplication>
#include "session.h"
#include "libnova/libnova.h"
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Wt;
using namespace GuLinux;

FindNeighbour& FindNeighbour::instance(Session& session)
{
  static FindNeighbour _find_neighbour(session);
  return _find_neighbour;
}

class FindNeighbour::Private {
public:
  Private(FindNeighbour *q);
    struct Object {
    Dbo::dbo_traits< NgcObject >::IdType id;
    ln_equ_posn coordinates;
  };
  std::list<Object> objects;
private:
  FindNeighbour *q;
};

FindNeighbour::Private::Private(FindNeighbour* q) : q{q}
{

}

FindNeighbour::FindNeighbour(Session& session) : dptr(this)
{
  benchmark loadObjects("LoadObjects", [](const string &name, int times, double time){ wApp->log("notice") << name << " took " << time << " msecs"; }, 1);
  Dbo::Transaction t(session);
  auto objects = session.find<NgcObject>().resultList();
  transform(begin(objects), end(objects), back_inserter(d->objects), [](const NgcObjectPtr &n){ return Private::Object{n.id(), {n->rightAscension().degrees(), n->declination().degrees()}}; });
  auto memory_size = sizeof(d->objects) + sizeof(Private::Object) * d->objects.size();
  wApp->log("notice") << "memory for objects: " << memory_size << " (" << static_cast<double>(memory_size) / 1024. / 1024. << " MB)";
}

FindNeighbour::~FindNeighbour()
{
}

vector<FindNeighbour::Neighbour> FindNeighbour::neighbours(const NgcObjectPtr& object, Dbo::Transaction& transaction, int page_limit, int page_number) const
{
  wApp->log("notice") << __PRETTY_FUNCTION__ << ": " << *object->objectId() << ", page_limit=" << page_limit << ", page_number=" << page_number;
  benchmark loadObjects("FindNeighbours", [](const string &name, int times, double time){ wApp->log("notice") << name << " took " << time << " msecs"; }, 1);
  struct NeighbourPrototype {
    Dbo::dbo_traits< NgcObject >::IdType id;
    Angle distance;
  };
  vector<NeighbourPrototype> nearby_objects;
  
  ln_equ_posn obj_pos{object->rightAscension().degrees(), object->declination().degrees()};
  for(auto o: d->objects) {
    auto separation = ln_get_angular_separation(&obj_pos, const_cast<ln_equ_posn*>(&o.coordinates));
    if(object.id() != o.id && separation < 25)
      nearby_objects.push_back({o.id, Angle::degrees(separation)});
  }
  
  sort(begin(nearby_objects), end(nearby_objects), [&obj_pos](const NeighbourPrototype &a, const NeighbourPrototype &b){
    return a.distance < b.distance;
  });

  wApp->log("notice") << "found " << nearby_objects.size() << " matching objects";
  if(page_limit > 0) {
    auto new_begin = begin(nearby_objects) + page_limit*page_number;
    nearby_objects = vector<NeighbourPrototype>(new_begin, new_begin + page_limit);
    wApp->log("notice") << "copy size: " << nearby_objects.size();
  }
  vector<Neighbour> results;
  transform(begin(nearby_objects), end(nearby_objects), back_inserter(results), [&transaction](const NeighbourPrototype &n){
    return Neighbour{n.distance, transaction.session().find<NgcObject>().where("id = ?").bind(n.id).resultValue() };
  });
  return results;
}

