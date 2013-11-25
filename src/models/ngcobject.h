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
#include <string>

class AstroSessionObject;
class NebulaDenomination;
namespace dbo = Wt::Dbo;

class NgcObject
{
public:
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
    NgcObject();
    ~NgcObject();
    // CREATE TABLE objects (object_id TEXT PRIMARY KEY, ra REAL, dec REAL, magnitude REAL, angular_size REAL, type INTEGER);
    std::string objectId() const;
    dbo::collection< dbo::ptr<NebulaDenomination> > nebulae() const;
    float rightAscension() const;
    float declination() const;
    float magnitude() const;
    float angularSize() const;
    NebulaType type() const;
    std::string typeDescription() const;
    template<class Action>
    void persist(Action& a) {
	dbo::id(a, _objectId, "object_id");
        dbo::field(a, _rightAscension, "ra");
        dbo::field(a, _declination, "dec");
        dbo::field(a, _magnitude, "magnitude");
        dbo::field(a, _angularSize, "angular_size");
        dbo::field(a, _type, "type");
	dbo::hasMany(a, _nebulae, dbo::ManyToOne);
	dbo::hasMany(a, _astroSessionObjects, dbo::ManyToOne);
    }
private:
    std::string _objectId;
    float _rightAscension, _declination, _magnitude, _angularSize;
    NebulaType _type;
    dbo::collection< dbo::ptr<NebulaDenomination> > _nebulae;
    dbo::collection< dbo::ptr<AstroSessionObject> > _astroSessionObjects;
};


namespace Wt {
    namespace Dbo {
        template<>
        struct dbo_traits<NgcObject> : public dbo_default_traits {
	    typedef std::string IdType;
            static IdType invalidId() {
                return std::string();
            }
            static const char *versionField() {
              return 0;
            }
            static const char *surrogateIdField() {
	      return 0;
	    }
        };

    }
}
#endif // NGCOBJECT_H
