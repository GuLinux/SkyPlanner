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

#ifndef CONSTELLATIONFINDER_H
#define CONSTELLATIONFINDER_H

#include <string>
#include "types.h"
class ConstellationFinder {
public:
  struct Constellation {
    std::string abbrev;
    std::string name;
    std::string genitive;
    operator bool() const { return !abbrev.empty() && !name.empty(); }
    bool operator==(const Constellation &o) { return o.abbrev == abbrev; }
    bool operator!=(const Constellation &o) { return o.abbrev != abbrev; }
  };
  static Constellation getName(double ra, double dec, double epoch = 2000.0);
  static Constellation getName(const Coordinates::Equatorial &coordinates, double epoch = 2000.0);
  static Constellation byAbbrev(const std::string &abbrev);
  static std::vector<Constellation> constellations();
};

#endif
