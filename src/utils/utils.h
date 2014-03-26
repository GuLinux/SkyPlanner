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

#ifndef UTILS_H
#define UTILS_H

#include <functional>
#include <ios>
#include <sstream>

class Utils
{
public:
  static int exponentialPercentage(double value, double limit, double base);
  template<typename T>
  static T fromHexString(const std::string &s) {
    std::stringstream ss;
    ss << std::hex << s;
    T t;
    ss >> t;
    return t;
  }
  static std::string sanitizeForURL(const std::string &in, const std::string &replacement = "-");
  static std::string csv(const std::string &value, char separator=',');
  static std::string mcg_name_fix(const std::string &src);
};

class Scope {
public:
  Scope(const std::function<void()> &f) : f(f) {};
  ~Scope() { f(); }
private:
  std::function<void()> f;
};

#endif // UTILS_H
