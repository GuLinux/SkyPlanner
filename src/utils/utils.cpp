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

#include "utils.h"
#include <cmath>
#include <boost/regex.hpp>
#include "utils/format.h"
#include <boost/algorithm/string.hpp>

using namespace std;

int Utils::exponentialPercentage( double value, double limit, double base )
{
  value = pow(base, value);
  limit = pow(base, limit);
  double percent = value * 100. /limit;
  return static_cast<int>(percent);
}


string Utils::sanitizeForURL(const string &in, const string &replacement)
{
  return boost::regex_replace(in, boost::regex("[^a-zA-Z0-9]+"), replacement );
}

string Utils::csv( const string &value, char separator )
{
  vector<char> toEscape{'\n', '\r', '"', separator};
  if(any_of(begin(toEscape), end(toEscape), [&value](char c) { return value.find(c) != string::npos; })) {
    return format("\"%s\"") % boost::algorithm::replace_all_copy(value, "\"", "\"\"");
  }
  return value;
}

string Utils::mcg_name_fix(const string &src)
{
  boost::regex r{"MCG([+-])(\\d+)-(\\d+)-(\\d+)(.*)"};
  boost::smatch what;
  if(!boost::regex_match(src, what, r)) return src;
  return format("MCG%s%02d-%02d-%03d%s") % what[1] % what[2] % what[3] % what[4] % what[5];
}