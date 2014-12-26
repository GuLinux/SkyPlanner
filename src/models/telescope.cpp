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

Eyepiece::Eyepiece(const string& name, int focalLength, int aFOV)
  : _name(name), _focalLength(focalLength), _aFOV(aFOV)
{

}

Eyepiece::Eyepiece()
{
}


int Eyepiece::aFOV() const
{
  return _aFOV;
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


