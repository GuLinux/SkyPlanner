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
#ifndef TELESCOPE_H
#define TELESCOPE_H
#include <Wt/Dbo/Dbo>

class User;
namespace dbo = Wt::Dbo;
class Telescope
{
public:
  explicit Telescope();
  explicit Telescope(const std::string &name, int diameter, int focalLength, bool isDefault = false);
  std::string name() const;
  int diameter() const;
  int focalLength() const;
  bool isDefault() const;
  void setDefault(bool _default);
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _name, "name");
    dbo::field(a, _diameter, "diameter");
    dbo::field(a, _focalLength, "focal_length");
    dbo::field(a, _default, "default");
    dbo::belongsTo(a, _user);
  }
  double limitMagnitudeGain() const;
private:
  std::string _name;
  int _diameter;
  int _focalLength;
  bool _default;
  dbo::ptr<User> _user;
};

typedef dbo::ptr<Telescope> TelescopePtr;

#endif // TELESCOPE_H
