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
#ifndef DBOHELPER_H
#define DBOHELPER_H
#include <Wt/Dbo/Query>
#include "widgets/skyobjects/astroobjectstable.h"
#include "Models"
#include <vector>
#include <boost/algorithm/string.hpp>

class DboHelper {
public:
  // Note: there must be an 'altitude' column, and the main table must be 'o'.
  template<typename T> 
  static Wt::Dbo::Query<T> filterQuery(Wt::Dbo::Transaction &transaction, const std::string &baseQuery, const AstroObjectsTable::Filters &filters, const AstroObjectsTable::Page &page = {}) {
    using namespace Wt;
    using namespace std;
    CataloguePtr catalogue = filters.catalogue;
    Dbo::Query<T> query = transaction.session().query<T>( catalogue ? baseQuery + " inner join denominations d on d.objects_id = o.id" : baseQuery);
    vector<string> filterConditions{filters.types.size(), "?"};
    query
      .where("magnitude >= ?").bind(filters.minimumMagnitude)
      .where("magnitude <= ?").bind(filters.maximumMagnitude);
    if(filters.minimumAltitude.valid())
      query.where("altitude >= ?").bind(filters.minimumAltitude.degrees());
    if(filters.maximumAltitude.valid())
      query.where("altitude <= ?").bind(filters.maximumAltitude.degrees());
    if(filters.minimumAngularSize.valid())
      query.where("angular_size >= ?").bind(filters.minimumAngularSize.degrees());
    if(filters.maximumAngularSize.valid())
      query.where("angular_size <= ?").bind(filters.maximumAngularSize.degrees());
    if(!filters.start_time.is_not_a_date_time() && !filters.end_time.is_not_a_date_time()) {
      query
	.where("transit_time >= ?").bind(filters.start_time)
	.where("transit_time <= ?").bind(filters.end_time);
    }
    if(catalogue)
      query.where("d.catalogues_id = ?").bind(catalogue.id());
    query.where(format("\"type\" IN (%s)") % boost::algorithm::join(filterConditions, ", ") );
    for(auto filter: filters.types)
      query.bind(filter);
    if(filters.constellation)
      query.where("constellation_abbrev = ?").bind(filters.constellation.abbrev);
    if(page)
      query.limit(page.pageSize).offset(page.pageSize * page.current);
    if( !boost::logic::indeterminate(filters.observed)) {
      query.where( format("(select count(*) from astro_session_object where objects_id = o.id AND observed = ?) %s 0") % (filters.observed ? ">" : "=") )
          .bind(true);
    }
    return query;
  }
};

#endif
