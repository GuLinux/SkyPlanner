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
#ifndef PTR_DEFS_H
#define PTR_DEFS_H


#include <Wt/Dbo/ptr>

#define add_ptr(Class) class Class;\
typedef Wt::Dbo::ptr<Class> Class ## Ptr;

add_ptr(AstroSession)
add_ptr(AstroSessionObject)
add_ptr(Telescope)
add_ptr(Eyepiece)
add_ptr(NgcObject)
add_ptr(NebulaDenomination)
add_ptr(ViewPort)
add_ptr(User)
add_ptr(Catalogue)
add_ptr(UserSetting)

#endif // PTR_DEFS_H
