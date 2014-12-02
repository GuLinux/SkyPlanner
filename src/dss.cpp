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
#include "dss.h"
#include <map>
#include <vector>
#include <algorithm>

using namespace std;
using namespace DSS;


static std::map<ImageVersion,std::string> imageVersionStrings{
  { poss2ukstu_red, "poss2ukstu_red" },
  { poss2ukstu_blue, "poss2ukstu_blue" },
  { poss2ukstu_ir, "poss2ukstu_ir" },
  { poss1_red, "poss1_red" },
  { poss1_blue, "poss1_blue" },
  { quickv, "quickv" },
  { phase2_gsc2, "phase2_gsc2" },
  { phase2_gsc1, "phase2_gsc1" },
};

ImageVersion DSS::imageVersion( const string &version )
{
  for(auto v: imageVersionStrings) {
    if(version == v.second)
      return v.first;
  }
  return phase2_gsc2;
}

string DSS::imageVersion( const ImageVersion &version )
{
  return imageVersionStrings[version];
}

vector< ImageVersion > DSS::versions()
{
  vector<ImageVersion> v;
  transform(begin(imageVersionStrings), end(imageVersionStrings), back_inserter(v), [](pair<ImageVersion,string> p) { return p.first; });
  return v;
}


