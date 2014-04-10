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

Ephemeris::BestAltitude AstroSessionObject::bestAltitude(const AstroSessionPtr &astroSession, const NgcObjectPtr &ngcObject, const Ephemeris &ephemeris, int rangeDeltaInHours)
{
  auto twilight = ephemeris.astronomicalTwilight(astroSession->date());
  return ephemeris.findBestAltitude( ngcObject->coordinates(), twilight.set, twilight.rise);
} 

Ephemeris::BestAltitude AstroSessionObject::bestAltitude(const Ephemeris &ephemeris, int rangeDeltaInHours) const
{
  return bestAltitude(_astroSession, _ngcObject, ephemeris, rangeDeltaInHours);
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

