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

#ifndef CURL_H
#define CURL_H

#include "c++/dptr.h"
#include <functional>
#include <map>

class Curl
{
  public:
    typedef std::function<int(double progress, double total, double percent)> ProgressFunc;
    Curl(std::ostream &output);
    ~Curl();
    
    Curl &setProgressCallback(ProgressFunc progressFunc);
    Curl &get(const std::string &url);
    bool requestOk();
    long httpResponseCode() const;
    std::string header(const std::string &name) const;
    std::map<std::string,std::string> headers() const;
    std::string contentType() const;
    std::string lastErrorMessage() const;

  private:
    D_PTR;
};

#endif // CURL_H
