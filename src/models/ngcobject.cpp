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
#include <map>

using namespace std;
NgcObject::NgcObject()
{
}

NgcObject::~NgcObject()
{
}

float NgcObject::angularSize() const
{
  return _angularSize;
}

float NgcObject::declination() const
{
  return _declination;
}

float NgcObject::magnitude() const
{
  return _magnitude;
}
Wt::Dbo::collection< Wt::Dbo::ptr< NebulaDenomination > > NgcObject::nebulae() const
{
  return _nebulae;
}
boost::optional<string> NgcObject::objectId() const
{
  return _objectId;
}
float NgcObject::rightAscension() const
{
  return _rightAscension;
}

Coordinates::Equatorial NgcObject::coordinates() const
{
  return { Angle::radians( rightAscension() ), Angle::radians( declination() ) };
}

int32_t NgcObject::difficulty( const Wt::Dbo::ptr< Telescope > &telescope ) const
{
  if( ! telescope || magnitude() > 90 )
    return -1;

  double magnitudeLimit = telescope->limitMagnitudeGain() + 6.5 ; // TODO: find proper value
  return Utils::exponentialPercentage( magnitude(), magnitudeLimit, 1.2 );
}


NgcObject::NebulaType NgcObject::type() const
{
  return _type;
}
/*
    enum NebulaType
    {
            NebGx=0,     //!< Galaxy
            NebOc=1,     //!< Open star cluster
            NebGc=2,     //!< Globular star cluster, usually in the Milky Way Galaxy
            NebN=3,      //!< Bright emission or reflection nebula
            NebPn=4,     //!< Planetary nebula
            NebDn=5,     //!< ??? Dark Nebula?      Does not exist in current catalog
            NebIg=6,     //!< ??? Irregular Galaxy? Does not exist in current catalog
            NebCn=7,     //!< Cluster associated with nebulosity
            NebUnknown=8 //!< Unknown type, catalog errors, "Unidentified Southern Objects" etc.
    };
*/
std::string NgcObject::typeDescriptionKey() const
{
  return typeDescriptionKey( type() );
}
std::string NgcObject::typeDescriptionKey( NebulaType nebulaType )
{
  static map<NebulaType, string> descriptions
  {
    {AllButStars, "ngcobject_type_AllButStars"},
    {All, "ngcobject_type_All"},
    {NebGx, "ngcobject_type_NebGx"},
    {NebOc, "ngcobject_type_NebOc"},
    {NebGc, "ngcobject_type_NebGc"},
    {NebN, "ngcobject_type_NebN"},
    {NebPn, "ngcobject_type_NebPn"},
    {NebDn, "ngcobject_type_NebDn"},
    {NebIg, "ngcobject_type_NebIg"},
    {NebCn, "ngcobject_type_NebCn"},
    {NebUnknown, "ngcobject_type_NebUnknown"},
    {NebGalCluster, "ngcobject_type_GalClust"},
    {Asterism, "ngcobject_type_Asterism"},
    {RedStar, "ngcobject_type_RedStar"},
  };
  return descriptions[nebulaType];
}

Wt::WString NgcObject::typeDescription( NebulaType nebulaType )
{
  return Wt::WString::tr( typeDescriptionKey( nebulaType ) );
}
Wt::WString NgcObject::typeDescription() const
{
  return typeDescription( type() );
}

vector<NebulaDenominationPtr> NgcObject::denominationsByCatalogueImportance() const
{
  vector<NebulaDenominationPtr> denominations {_nebulae.begin(), _nebulae.end()};
  static map<string, int> catalogRatings
  {
    {"Messier", -99},
    {"NGC", -98},
    {"IC", -97},
    {"Arp", -96},
    {"Caldwell", -95},
    {"Abell", -94},
    {"UGC", -93},
    {"MCG", -92},
  };
  sort( denominations.begin(), denominations.end(), []( const NebulaDenominationPtr & a, const NebulaDenominationPtr & b )
  {
    if( !a->catalogue() && ! b->catalogue() )
      return a->name() < b->name();

    if( !a->catalogue() )
      return true;

    if( !b->catalogue() )
      return false;

    return catalogRatings[*a->catalogue()] < catalogRatings[*b->catalogue()];
  } );
  return denominations;
}

vector< string > NgcObject::namesByCatalogueImportance() const
{
  vector<string> names;
  auto denominations = denominationsByCatalogueImportance();
  for(auto denomination: denominations) {
      if(std::count(names.begin(), names.end(), denomination->name()) == 0)
        names.push_back(denomination->name());
  }
  return names;
}



