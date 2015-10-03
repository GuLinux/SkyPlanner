/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#ifndef FINDNEIGHBOUR_H
#define FINDNEIGHBOUR_H
#include "c++/dptr.h"
#include "models/Models"

class Session;
class FindNeighbour
{
public:
  FindNeighbour(Session& session);
  ~FindNeighbour();
  static FindNeighbour &instance(Session& session);
  struct Neighbour {
    Angle distance;
    NgcObjectPtr object;
  };
  std::vector<Neighbour> neighbours(const NgcObjectPtr& object, Wt::Dbo::Transaction& transaction, int page_limit = -1, int page_number = 0) const;
private:
  D_PTR
};

#endif // FINDNEIGHBOUR_H
