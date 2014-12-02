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
#ifndef NGCOBJECT_H
#define NGCOBJECT_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WString>
#include <string>
#include "constellationfinder.h"
#include "types.h"
#include "ptr_defs.h"
#include <Wt-Commons/json_object.h>

class Telescope;
class AstroSessionObject;
class NebulaDenomination;
namespace dbo = Wt::Dbo;
namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<NgcObject> : public dbo_default_traits {
            static const char *versionField() {
              return 0;
            }
        };

    }
}
class NgcObject : public WtCommons::Json::Object
{
public:
    enum NebulaType
    {
            NebGx=0x0,     //!< Galaxy
            NebOc=0x1,     //!< Open star cluster
            NebGc=0x2,     //!< Globular star cluster, usually in the Milky Way Galaxy
            NebN=0x3,      //!< Bright emission or reflection nebula
            NebPn=0x4,     //!< Planetary nebula
            NebDn=0x5,     //!< ??? Dark Nebula?      Does not exist in current catalog
            NebIg=0x6,     //!< ??? Irregular Galaxy? Does not exist in current catalog
            NebCn=0x7,     //!< Cluster associated with nebulosity
            NebUnknown=0x8, //!< Unknown type, catalog errors, "Unidentified Southern Objects" etc.
            NebGalCluster=0x9, //!< Galaxy Clusters, mostly abell
            Asterism=0xA,
            RedStar=0xB,
            NebGalGroups=0xC,
    };
    static const int NebulaTypeCount = 13;
    static std::set<NebulaType> allNebulaTypes();
    static std::set<NebulaType> allNebulaTypesButStars();
    NgcObject();
    ~NgcObject();
    boost::optional<std::string> objectId() const;
    dbo::collection<NebulaDenominationPtr> nebulae() const;
    double rightAscension() const;
    double declination() const;
    Coordinates::Equatorial coordinates() const;
    double magnitude() const;
    double angularSize() const;
    NebulaType type() const;
    static std::string typeDescriptionKey(NebulaType type);
    static Wt::WString typeDescription(NebulaType type);
    std::string typeDescriptionKey() const;
    Wt::WString typeDescription() const;
    int32_t difficulty(const dbo::ptr<Telescope> &telescope) const;

    template<class Action>
    void persist(Action& a) {
        dbo::field(a, _objectId, "object_id");
        dbo::field(a, _rightAscension, "ra");
        dbo::field(a, _declination, "dec");
        dbo::field(a, _magnitude, "magnitude");
        dbo::field(a, _angularSize, "angular_size");
        dbo::field(a, _type, "type");
        dbo::field(a, _constellationAbbrev, "constellation_abbrev");
        dbo::field(a, _extraData, "extra_data");
        dbo::hasMany(a, _nebulae, dbo::ManyToOne);
        dbo::hasMany(a, _astroSessionObjects, dbo::ManyToOne);
    }
    static std::vector<NebulaDenominationPtr> denominationsByCatalogueImportance(dbo::Transaction &transaction, const NgcObjectPtr &object);
    static std::vector<std::string> namesByCatalogueImportance(dbo::Transaction &transaction, const NgcObjectPtr &object);

    struct CatalogueDescription {
      CataloguePtr catalogue;
      std::string description;
    };

    std::vector<CatalogueDescription> descriptions() const;
    ConstellationFinder::Constellation constellation() const;
    void updateConstellation();
    NgcObject(Angle rightAscension, Angle declination, double magnitude, Angle angularSize, NebulaType nebulaType,
              const boost::optional<std::string> &objectId = boost::optional<std::string>(),
              const boost::optional<std::string> &constellationAbbrev = boost::optional<std::string>(),
              const boost::optional<std::string> &extraData = boost::optional<std::string>());
    virtual void add_to_json(Wt::Json::Object& object) const;
private:
    boost::optional<std::string> _objectId;
    double _rightAscension, _declination, _magnitude, _angularSize;
    NebulaType _type;
    dbo::collection<NebulaDenominationPtr> _nebulae;
    dbo::collection<NebulaDenominationPtr> _astroSessionObjects;
    dbo::dbo_traits<NgcObject>::IdType _id;
    boost::optional<std::string> _constellationAbbrev;
    boost::optional<std::string> _extraData;
    void mapJsonFields();
};


#endif // NGCOBJECT_H
