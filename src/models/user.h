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
#ifndef USER_H
#define USER_H
#include <Wt/Auth/Dbo/AuthInfo>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <string>
#include "ptr_defs.h"

namespace dbo = Wt::Dbo;

typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;
class User {
public:
  template<class Action>
  void persist(Action& a)
  {
    dbo::field(a, _isAdmin, "is_admin");
    dbo::field(a, _banned, "banned");
    dbo::hasMany(a, _telescopes, dbo::ManyToOne);
    dbo::hasMany(a, _eyepieces, dbo::ManyToOne);
    dbo::hasMany(a, _focalModifiers, dbo::ManyToOne);
    dbo::hasMany(a, _astroSessions, dbo::ManyToOne);
    dbo::hasMany(a, _settings, dbo::ManyToOne);
    dbo::hasOne(a, _authInfo);
  }
  dbo::collection<TelescopePtr> telescopes() const;
  dbo::collection<EyepiecePtr> eyepieces() const;
  dbo::collection<FocalModifierPtr> focalModifiers() const;
  std::list<FocalModifierPtr> focalModifiers(const std::string &idempotentLabel);
  dbo::collection<AstroSessionPtr> astroSessions() const;
  dbo::weak_ptr<AuthInfo> authInfo() const;
  Wt::WString loginName() const;
  bool isAdmin() const;
  bool banned() const;
  class Setting;
  typedef dbo::ptr<Setting> SettingPtr;
private:
  dbo::collection<TelescopePtr> _telescopes;
  dbo::collection<EyepiecePtr> _eyepieces;
  dbo::collection<FocalModifierPtr> _focalModifiers;
  dbo::collection<AstroSessionPtr> _astroSessions;
  bool _isAdmin = false;
  bool _banned = false;

  friend class Setting;
  dbo::collection<dbo::ptr<Setting>> _settings;
  dbo::weak_ptr<AuthInfo> _authInfo;
};

typedef dbo::ptr<User> UserPtr;
#endif // USER_H
