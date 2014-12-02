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
#ifndef USERSETTING_H
#define USERSETTING_H
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <string>
#include "user.h"
namespace dbo = Wt::Dbo;
class User;

class User::Setting
{
public:
  struct Id {
  std::string name;
  dbo::ptr<User> user;
  bool operator==(const Id &o) const {
    return name == o.name && user == o.user;
  }
  bool operator <(const Id &o) const {
    if(name < o.name)
      return true;
    else if(user < o.user)
      return true;
    return false;
  }
};
  
  Setting();
  Setting(const dbo::ptr<User> &user, const std::string &name, const std::string &value);
  template<class Action>
  void persist(Action& a);

  template<typename T> static T value(dbo::Transaction &transaction, const std::string &key, const dbo::ptr<User> &user, const T &defaultValue = T());
  template<typename T> static void setValue(dbo::Transaction &transaction, const std::string &key, const dbo::ptr<User> &user, const T &value);

private:
  static dbo::ptr<Setting> find(const std::string &name, const dbo::ptr<User> &user, dbo::Transaction &transaction);
  static void saveOrUpdate(const std::string &name, const std::string &value, dbo::ptr<User> user, dbo::Transaction &transaction);
  std::string _value;
  Id _id;
};

namespace Wt {
  namespace Dbo {

    template <class Action>
    void field(Action& action, User::Setting::Id& id, const std::string& name, int size = -1)
    {
      belongsTo(action, id.user, "user");
      field(action, id.name, name + "_name");
    }
    template<>
    struct dbo_traits<User::Setting> : public dbo_default_traits
    {
      typedef User::Setting::Id IdType;
      static IdType invalidId() { return IdType(); }
      static const char *surrogateIdField() { return 0; }
    };
  }
}

std::ostream &operator<<(std::ostream &o, const User::Setting::Id &id);

template<typename Action> void User::Setting::persist(Action& a)
{
  dbo::id(a, _id);
  dbo::field(a, _value, "value");
}



template<typename T> T User::Setting::value(dbo::Transaction &transaction, const std::string &key, const dbo::ptr<User> &user, const T &defaultValue) {
  auto found = find(key, user, transaction);
  return found ? boost::lexical_cast<T>(found->_value) : defaultValue;
}

template<typename T> void User::Setting::setValue(dbo::Transaction &transaction, const std::string &key, const dbo::ptr<User> &user, const T &value) {
  saveOrUpdate(key, boost::lexical_cast<std::string>(value), user, transaction);
}
#endif // USERSETTING_H
