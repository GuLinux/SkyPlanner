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
#ifndef ASTRO_GROUP_H
#define ASTRO_GROUP_H
#include "Models"

#include "types.h"

struct AstroGroup {
  AstroSessionPtr _astroSession;
  NgcObjectPtr _object;
  AstroSessionObjectPtr astroSessionObject;
  TelescopePtr telescope;
  Timezone timezone;

  AstroGroup(const AstroSessionPtr &astroSession, const TelescopePtr &telescope = {}, const Timezone &timezone = {})
    : _astroSession(astroSession), telescope(telescope), timezone(timezone) {}
  AstroGroup(const AstroSessionPtr &astroSession, const NgcObjectPtr &object, const TelescopePtr &telescope = {}, const Timezone &timezone = {})
    : _astroSession(astroSession), _object(object), telescope(telescope), timezone(timezone) {}
  AstroGroup(const AstroSessionObjectPtr &astroSessionObject, const TelescopePtr &telescope = {}, const Timezone &timezone = {})
    : astroSessionObject(astroSessionObject),telescope(telescope), timezone(timezone) {}

  AstroSessionPtr astroSession() const;
  NgcObjectPtr object() const;
};


#endif // ASTRO_GROUP_H
