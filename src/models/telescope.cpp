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

using namespace std;

Telescope::Telescope()
{
}

Telescope::Telescope( const string &name, int diameter, int focalLength, bool isDefault )
  : _name(name), _diameter(diameter), _focalLength(focalLength), _default(isDefault)
{
  
}


int Telescope::diameter() const
{
  return _diameter;
}

int Telescope::focalLength() const
{
  return _focalLength;
}

double Telescope::limitMagnitudeGain() const
{
  return 5.0 * log10( static_cast<double>(diameter()) / 6.5 );
}

string Telescope::name() const
{
  return _name;
}

bool Telescope::isDefault() const
{
  return _default;
}

void Telescope::setDefault(bool _default)
{
  this->_default = _default;
}

Eyepiece::Eyepiece(const string& name, int focalLength, Angle aFOV)
  : _name(name), _focalLength(focalLength), _aFOV(aFOV.degrees())
{

}

Eyepiece::Eyepiece()
{
}


Angle Eyepiece::aFOV() const
{
  return Angle::degrees(_aFOV);
}

int Eyepiece::focalLength() const
{
  return _focalLength;
}

string Eyepiece::name() const
{
  return _name;
}


FocalModifier::FocalModifier(const string& name, double ratio)
  : _name(name), _ratio(ratio)
{
}

FocalModifier::FocalModifier()
{
}

string FocalModifier::name() const
{
  return _name;
}

double FocalModifier::ratio() const
{
  return _ratio;
}

OpticalSetup::OpticalSetup(const TelescopePtr& telescope, const EyepiecePtr& eyepiece, const FocalModifierPtr &focalModifier)
  : _telescope(telescope), _eyepiece(eyepiece), _focalModifier(focalModifier)
{
}

EyepiecePtr OpticalSetup::eyepiece() const
{
  return _eyepiece;
}

OpticalSetup& OpticalSetup::eyepiece(const EyepiecePtr& eyepiece)
{
  _eyepiece = eyepiece;
  return *this;
}

TelescopePtr OpticalSetup::telescope() const
{
  return _telescope;
}

OpticalSetup& OpticalSetup::telescope(const TelescopePtr& telescope)
{
  _telescope = telescope;
  return *this;
}

FocalModifierPtr OpticalSetup::focalModifier() const
{
  return _focalModifier;
}

OpticalSetup& OpticalSetup::focalModifier(const FocalModifierPtr& focalModifier)
{
  _focalModifier = focalModifier;
  return *this;
}


Angle OpticalSetup::fov() const
{
  return _eyepiece->aFOV() / magnification();
}

double OpticalSetup::magnification() const
{
  if(!_telescope || !_eyepiece) {
    throw runtime_error("You must add an eyepiece and a telescope");
  }
  double multiplier = _focalModifier ? _focalModifier->ratio() : 1.;
  return static_cast<double>(_telescope->focalLength()) / static_cast<double>(_eyepiece->focalLength() ) * multiplier;
}
