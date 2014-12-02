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
#include "Models"
#include "usersetting.h"
using namespace Wt;
using namespace std;

User::Setting::Setting()
{
}

User::Setting::Setting(const UserPtr &user, const string &name, const string &value)
  : _id({name, user}), _value(value)
{
}

ostream& operator<< (ostream& o, const User::Setting::Id& id) {
  return o << "[user=" << id.user.id() << ", name=" << id.name << "]";
}

User::SettingPtr User::Setting::find(const string &name, const UserPtr &user, dbo::Transaction &transaction) {
  auto found = find_if(begin(user->_settings), end(user->_settings), [name](const User::SettingPtr &s) { return s && s->_id.name == name; });
  if(found != end(user->_settings))
    return *found;
  return User::SettingPtr();
}

void User::Setting::saveOrUpdate(const std::string &name, const std::string &value, UserPtr user, dbo::Transaction &transaction) {
  if(User::SettingPtr found = find(name, user, transaction)) {
    user.modify()->_settings.erase(found);
    found.remove();
  }
  user.modify()->_settings.insert(new User::Setting(user, name, value));
  user.flush();
}
