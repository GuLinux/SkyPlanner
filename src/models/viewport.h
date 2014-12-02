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
#ifndef VIEWPORT_H
#define VIEWPORT_H
#include <Wt/Dbo/Dbo>
#include "types.h"
#include "dss.h"

namespace dbo = Wt::Dbo;
class User;
class NgcObject;

class ViewPort
{
public:
    explicit ViewPort() = default;
    explicit ViewPort(const Coordinates::Equatorial &coordinates, const Angle &angle, DSS::ImageVersion imageVersion, const dbo::ptr<NgcObject> &ngcObject, const dbo::ptr<User> &user);


  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, ar, "ar");
    dbo::field(a, dec , "dec");
    dbo::field(a, arcminutes, "angular_size");
    dbo::field(a, _imageVersion, "image_version");
    dbo::belongsTo(a, _user);
    dbo::belongsTo(a, _ngcObject);
  }
  Angle angularSize() const;
  Coordinates::Equatorial coordinates() const;
  static Angle defaultAngle(const dbo::ptr<NgcObject> &ngcObject, dbo::Transaction &transaction);
  static ViewPort findOrCreate(DSS::ImageVersion imageVersion, const dbo::ptr<NgcObject> &ngcObject, const dbo::ptr<User> &user, dbo::Transaction &transaction);
  static void setImageVersion(DSS::ImageVersion imageVersion, const dbo::ptr<NgcObject> &ngcObject, const dbo::ptr<User> &user, dbo::Transaction &transaction);
  static void save(const Coordinates::Equatorial &coordinates, const Angle &angularSize, DSS::ImageVersion imageVersion, const dbo::ptr<NgcObject> &ngcObject, const dbo::ptr<User> &user, dbo::Transaction &transaction);
  DSS::ImageVersion imageVersion() const;
  void reset(const dbo::ptr<NgcObject> &ngcObject, dbo::Transaction &transaction);
private:
  double arcminutes;
  double ar;
  double dec;
  DSS::ImageVersion _imageVersion;
  dbo::ptr<NgcObject> _ngcObject;
  dbo::ptr<User> _user;
};


namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<ViewPort> : public dbo_default_traits {
//      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
    };

  }
}

typedef dbo::ptr<ViewPort> ViewPortPtr;

#endif // VIEWPORT_H
