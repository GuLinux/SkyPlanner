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
