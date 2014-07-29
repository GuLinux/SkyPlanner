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

#include "Models"
#include <utils/utils.h>
#include <Wt/Dbo/Transaction>
#include <Wt/Dbo/Session>
#include <Wt/WServer>

using namespace Wt;
using namespace std;

AstroSessionObject::AstroSessionObject()
{

}

AstroSessionObject::AstroSessionObject(const Wt::Dbo::ptr< NgcObject >& ngcObject)
  : _ngcObject(ngcObject)
{

}

NgcObjectPtr AstroSessionObject::ngcObject() const
{
  return _ngcObject;
}

AstroSessionPtr AstroSessionObject::astroSession() const
{
  return _astroSession;
}

Coordinates::Equatorial AstroSessionObject::coordinates() const
{
  return _ngcObject->coordinates();
}

Ephemeris::BestAltitude AstroSessionObject::bestAltitude(const AstroSessionPtr &astroSession, const NgcObjectPtr &ngcObject, const Ephemeris &ephemeris)
{
  auto twilight = ephemeris.astronomicalTwilight(astroSession->date());
  return ephemeris.findBestAltitude( ngcObject->coordinates(), twilight.set, twilight.rise);
}

void AstroSessionObject::generateEphemeris(const Ephemeris &ephemeris, const AstroSessionPtr &astroSession, const Timezone &timezone, Dbo::Transaction &transaction)
{
  string cacheKey = transaction.session().query<string>(R"(SELECT CAST("when" as TEXT) || '-LAT' || CAST(latitude as TEXT) || '-LNG'|| CAST(longitude as TEXT) FROM astro_session WHERE id = ?)")
      .bind(astroSession.id());
  WServer::instance()->log("notice") << __PRETTY_FUNCTION__ << ": got astrosession location/date key: " << cacheKey;
  auto objects = transaction.session().find<AstroSessionObject>().where("astro_session_id = ? AND ( \
    ephemeris_context_key IS NULL OR ephemeris_context_key <>  ? OR transit_time is null OR altitude is null OR azimuth is null) ").bind(astroSession.id()).bind(cacheKey).resultList();

  WServer::instance()->log("notice") << __PRETTY_FUNCTION__ << ": objects count: " << objects.size();

  for(auto object: objects) {
    auto bestAltitude = object->bestAltitude(ephemeris, timezone);
    object.modify()->_transitTime.reset(bestAltitude.when.utc);
    object.modify()->_altitude.reset(bestAltitude.coordinates.altitude.degrees());
    object.modify()->_azimuth.reset(bestAltitude.coordinates.azimuth.degrees());
    object.modify()->_ephemeris_context_key.reset(cacheKey);
  }
}

Ephemeris::BestAltitude AstroSessionObject::bestAltitude(const Ephemeris &ephemeris, const Timezone &timezone) const
{
  if(_transitTime && _altitude && _azimuth)
    return { {Angle::degrees(*_altitude), Angle::degrees(*_azimuth)},  DateTime::fromUTC(*_transitTime, timezone)};
  return bestAltitude(_astroSession, _ngcObject, ephemeris);
}


int32_t AstroSessionObject::difficulty( const Wt::Dbo::ptr< Telescope > &telescope ) const
{
  if(! telescope || _ngcObject->magnitude() > 90)
    return -1;
  double magnitudeLimit = telescope->limitMagnitudeGain() + 6.5 ; // TODO: find best value.
  return Utils::exponentialPercentage(_ngcObject->magnitude(), magnitudeLimit, 1.2);
}

void AstroSessionObject::setDescription(const string &description)
{
  _description = description;
}

string AstroSessionObject::description() const
{
  return _description;
}

bool AstroSessionObject::observed() const
{
  return _observed;
}

void AstroSessionObject::setObserved(bool observed)
{
  _observed = observed;
}

boost::optional< string > AstroSessionObject::report() const
{
  return _report;
}
void AstroSessionObject::setReport(const string& report)
{
  _report = report;
}
