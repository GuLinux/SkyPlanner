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
#include "ptr_defs.h"
#include <types.h>

namespace dbo = Wt::Dbo;

class Eyepiece {
public:
  explicit Eyepiece();
  explicit Eyepiece(const std::string &name, int focalLength, Angle aFOV);
  std::string name() const;
  int focalLength() const;
  Angle aFOV() const;
  
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _name, "name");
    dbo::field(a, _focalLength, "focal_length");
    dbo::field(a, _aFOV, "afov");
    dbo::belongsTo(a, _user);
  }
private:
  std::string _name;
  int _focalLength;
  int _aFOV;
  dbo::ptr<User> _user;
};

class FocalModifier {
public:
  explicit FocalModifier();
  explicit FocalModifier(const std::string &name, double ratio);
  std::string name() const;
  double ratio() const;
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _name, "name");
    dbo::field(a, _ratio, "ratio");
    dbo::belongsTo(a, _user);
  }
private:
  std::string _name;
  double _ratio;
  dbo::ptr<User> _user;
};

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

class OpticalSetup {
public:
  OpticalSetup(const TelescopePtr &telescope = {}, const EyepiecePtr &eyepiece = {}, const FocalModifierPtr &focalModifier = {} );
  OpticalSetup &telescope(const TelescopePtr &telescope);
  OpticalSetup &eyepiece(const EyepiecePtr &eyepiece);
  OpticalSetup &focalModifier(const FocalModifierPtr& focalModifier);
  
  TelescopePtr telescope() const;
  EyepiecePtr eyepiece() const;
  FocalModifierPtr focalModifier() const;
  
  double magnification() const;
  Angle fov() const;
private:
  TelescopePtr _telescope;
  EyepiecePtr _eyepiece;
  FocalModifierPtr _focalModifier;
};


#endif // TELESCOPE_H
