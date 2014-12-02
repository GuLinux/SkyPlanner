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
#ifndef DSS_NAMESPACE
#define DSS_NAMESPACE
#include <map>
#include <string>
#include <vector>

namespace DSS {
    enum ImageVersion {
      poss2ukstu_red,
      poss2ukstu_blue,
      poss2ukstu_ir,
      poss1_red,
      poss1_blue,
      quickv,
      phase2_gsc2,
      phase2_gsc1,
    };

    DSS::ImageVersion imageVersion(const std::string &version);
    std::string imageVersion(const DSS::ImageVersion &version);
    std::vector<DSS::ImageVersion> versions();

}

#endif

