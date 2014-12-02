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

class AstroSession;
class AstroSessionObject;
class Telescope;
class NgcObject;
class NebulaDenomination;
class ViewPort;
class User;
class Catalogue;
class UserSetting;

typedef Wt::Dbo::ptr<AstroSession> AstroSessionPtr;
typedef Wt::Dbo::ptr<AstroSessionObject> AstroSessionObjectPtr;
typedef Wt::Dbo::ptr<Telescope> TelescopePtr;
typedef Wt::Dbo::ptr<NgcObject> NgcObjectPtr;
typedef Wt::Dbo::ptr<NebulaDenomination> NebulaDenominationPtr;
typedef Wt::Dbo::ptr<ViewPort> ViewPortPtr;
typedef Wt::Dbo::ptr<User> UserPtr;
typedef Wt::Dbo::ptr<Catalogue> CataloguePtr;
typedef Wt::Dbo::ptr<UserSetting> UserSettingPtr;

#endif // PTR_DEFS_H
