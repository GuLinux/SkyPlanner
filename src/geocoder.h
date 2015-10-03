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
#ifndef GEOCODER_H
#define GEOCODER_H
#include "types.h"
#include "c++/dptr.h"

class GeoCoder {
public:
  struct Place {
    std::string city;
    std::string address;
    std::string province;
    std::string provinceCode;
    std::string zipCode;
    std::string region;
    std::string country;
    std::string formattedAddress;
    operator bool() const;
  };
  GeoCoder(const std::string &apiKey);
  ~GeoCoder();
  Place reverse(const Coordinates::LatLng &latlng) const;
  struct PlaceInformation {
    Timezone timezone;
    GeoCoder::Place geocoderPlace;
  };
  static PlaceInformation placeInformation(const Coordinates::LatLng &coordinates, const boost::posix_time::ptime &when);
  private:
    D_PTR
};

std::ostream &operator<<(std::ostream&, const GeoCoder::Place &);
#endif