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
#ifndef GUM_FORMAT_H
#define GUM_FORMAT_H
#include <boost/format.hpp>
#include <Wt/WString>

class format {
public:
  format(const std::string &formatString) : _format(formatString) {}
  operator std::string() { return str(); }
  operator Wt::WString() { return wtstr(); }
  template<typename FormatValue>
  format &operator%(FormatValue formatValue) { _format % formatValue;  return *this; }
  std::string str() const { return _format.str(); }
  Wt::WString wtstr() const { return Wt::WString::fromUTF8(_format.str()); }
private:
  boost::format _format;
};

#endif