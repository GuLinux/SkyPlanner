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
#include "Models"
#include <utils/utils.h>
#include <map>
#include <boost/algorithm/string/trim.hpp>

using namespace std;

NgcObject::NgcObject(Angle rightAscension, Angle declination, double magnitude, Angle angularSize, NgcObject::NebulaType nebulaType, const boost::optional< string >& objectId, const boost::optional< string >& constellationAbbrev, const boost::optional< string >& extraData)
  : _rightAscension(rightAscension.radians() ), _declination(declination.radians() ), _magnitude(magnitude), _angularSize(angularSize.degrees() ),
    _type(nebulaType), _objectId(objectId), _constellationAbbrev(constellationAbbrev), _extraData(extraData)
{
  mapJsonFields();
}

NgcObject::NgcObject()
{
  mapJsonFields();
}

void NgcObject::mapJsonFields()
{
  addField<double>("ar", _rightAscension);
  addField<double>("dec", _declination);
  addField<double>("magnitude", _magnitude);
  addField<double>("angular_size", _angularSize);
}

void NgcObject::add_to_json(Wt::Json::Object& object) const
{
    object["type"] = static_cast<int>(_type);
    object["type_description"] = Wt::WString::fromUTF8(typeDescriptionKey());
    if(_objectId)
      object["object_id"] = Wt::WString::fromUTF8(*_objectId);
}


NgcObject::~NgcObject()
{
}

double NgcObject::angularSize() const
{
  return _angularSize;
}

Angle NgcObject::declination() const
{
  return Angle::radians(_declination);
}

double NgcObject::magnitude() const
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
Angle NgcObject::rightAscension() const
{
  return Angle::radians(_rightAscension);
}

Coordinates::Equatorial NgcObject::coordinates() const
{
  return { rightAscension(), declination() };
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
string NgcObject::typeDescriptionKey() const
{
  return typeDescriptionKey( type() );
}
string NgcObject::typeDescriptionKey( NebulaType nebulaType )
{
  static map<NebulaType, string> descriptions
  {
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
    {NebGalGroups, "ngcobject_type_NebGalGroups"},
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

vector< Wt::Dbo::ptr< NebulaDenomination > > NgcObject::denominationsByCatalogueImportance(Wt::Dbo::Transaction &transaction , const NgcObjectPtr &object)
{
  dbo::collection<NebulaDenominationPtr> denominationsByPriority =
    transaction.session().query<NebulaDenominationPtr>("SELECT d from denominations d inner join catalogues on d.catalogues_id = catalogues.id \
    WHERE d.objects_id = ? \
    ORDER BY priority ASC").bind(object.id());
  vector<NebulaDenominationPtr> denominations {denominationsByPriority.begin(), denominationsByPriority.end()};
  return denominations;
}

vector< string > NgcObject::namesByCatalogueImportance( Wt::Dbo::Transaction &transaction, const NgcObjectPtr &object )
{
  vector<string> names;
  vector<NebulaDenominationPtr> denominations = denominationsByCatalogueImportance(transaction, object);
  for(NebulaDenominationPtr d: denominations) {
      if(count(names.begin(), names.end(), d->name()) == 0)
        names.push_back( d->name());
  }
  return names;
}

set< NgcObject::NebulaType > NgcObject::allNebulaTypes()
{
  static set<NgcObject::NebulaType> nebulaTypes;
  if(nebulaTypes.empty())
    for(int i=NebGx; i<NebulaTypeCount; i++)
      nebulaTypes.insert(static_cast<NebulaType>(i));
  return nebulaTypes;
}

set< NgcObject::NebulaType > NgcObject::allNebulaTypesButStars()
{
  auto nebulaTypes = allNebulaTypes();
  nebulaTypes.erase(Asterism);
  nebulaTypes.erase(RedStar);
  return nebulaTypes;
}

vector<NgcObject::CatalogueDescription> NgcObject::descriptions() const
{
  vector<CatalogueDescription> dbDescriptions;
  vector<NebulaDenominationPtr> denominations;
  copy_if(begin(_nebulae), end(_nebulae), back_inserter(denominations),
          [](const NebulaDenominationPtr &d){ return d->comment() && ! boost::algorithm::trim_copy(*d->comment()).empty(); });

  transform(begin(denominations), end(denominations), back_inserter(dbDescriptions), [](const NebulaDenominationPtr &d){ return CatalogueDescription{d->catalogue(), *d->comment()}; });
  return dbDescriptions;
}

void NgcObject::updateConstellation()
{
  ConstellationFinder::Constellation constellation = ConstellationFinder::getName(coordinates());
  if(constellation)
      _constellationAbbrev = constellation.abbrev;
}

ConstellationFinder::Constellation NgcObject::constellation() const
{
  if(!_constellationAbbrev) return ConstellationFinder::Constellation{};
  return ConstellationFinder::byAbbrev(*_constellationAbbrev);
}

NgcObject::operator Wt::Json::Object() const
{
  Wt::Json::Object object;
  object["object-id"] = {_objectId ? *_objectId : ""};
  object["angular-size"] = {_angularSize};
  object["magnitude"] = {_magnitude};
  object["right-ascension"] = {_rightAscension};
  object["declination"] = {_declination};
  object["type"] = {_type};
  object["extra-data"] = {_extraData ? *_extraData : ""};
  return object;
}

