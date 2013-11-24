/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

Telescope::Telescope( const string &name, int diameter, int focalLength )
  : _name(name), _diameter(diameter), _focalLength(focalLength)
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
