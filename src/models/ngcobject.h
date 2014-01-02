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

#ifndef NGCOBJECT_H
#define NGCOBJECT_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WString>
#include <string>
#include "types.h"

class Telescope;
class AstroSessionObject;
class NebulaDenomination;
namespace dbo = Wt::Dbo;

class NgcObject
{
public:
    enum NebulaType
    {
            All=-1,
            NebGx=0x0,     //!< Galaxy
            NebOc=0x1,     //!< Open star cluster
            NebGc=0x2,     //!< Globular star cluster, usually in the Milky Way Galaxy
            NebN=0x3,      //!< Bright emission or reflection nebula
            NebPn=0x4,     //!< Planetary nebula
            NebDn=0x5,     //!< ??? Dark Nebula?      Does not exist in current catalog
            NebIg=0x6,     //!< ??? Irregular Galaxy? Does not exist in current catalog
            NebCn=0x7,     //!< Cluster associated with nebulosity
            NebUnknown=0x8, //!< Unknown type, catalog errors, "Unidentified Southern Objects" etc.
            NebGalCluster=0x9 //!< Galaxy Clusters, mostly abell
    };
    static const int NebulaTypeCount = 9;
    NgcObject();
    ~NgcObject();
    // CREATE TABLE objects (object_id TEXT PRIMARY KEY, ra REAL, dec REAL, magnitude REAL, angular_size REAL, type INTEGER);
    boost::optional<std::string> objectId() const;
    dbo::collection< dbo::ptr<NebulaDenomination> > nebulae() const;
    float rightAscension() const;
    float declination() const;
    Coordinates::Equatorial coordinates() const;
    float magnitude() const;
    float angularSize() const;
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
	dbo::hasMany(a, _nebulae, dbo::ManyToOne);
	dbo::hasMany(a, _astroSessionObjects, dbo::ManyToOne);
    }
private:
    boost::optional<std::string> _objectId;
    float _rightAscension, _declination, _magnitude, _angularSize;
    NebulaType _type;
    dbo::collection< dbo::ptr<NebulaDenomination> > _nebulae;
    dbo::collection< dbo::ptr<AstroSessionObject> > _astroSessionObjects;
};

typedef dbo::ptr<NgcObject> NgcObjectPtr;

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
#endif // NGCOBJECT_H
