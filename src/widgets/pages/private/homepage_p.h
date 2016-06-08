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

#ifndef HOMEPAGE_P_H
#define HOMEPAGE_P_H
#include "widgets/pages/homepage.h"

namespace Wt
{
  class WTemplate;
}

class HomePage::Private
{
  public:
    Private( Session &session, HomePage *q );
    Session &session;
    Wt::JSignal<std::string> link;
    Wt::WTemplate *content;
    void populate();
  private:
    class HomePage *const q;
};
#endif // HOMEPAGE_P_H
