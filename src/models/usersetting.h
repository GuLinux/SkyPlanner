#ifndef USERSETTING_H
#define USERSETTING_H
#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <string>
#include "user.h"
namespace dbo = Wt::Dbo;
class User;

struct UserSettingId {
  std::string name;
  dbo::ptr<User> user;
  bool operator==(const UserSettingId &o) const {
    return name == o.name && user == o.user;
  }
  bool operator <(const UserSettingId &o) const {
    if(name < o.name)
      return true;
    else if(user < o.user)
      return true;
    return false;
  }
};

std::ostream &operator<<(std::ostream &o, const UserSettingId &id);

namespace Wt {
  namespace Dbo {
    template<>
    struct dbo_traits<User::Setting> : public dbo_default_traits
    {
      typedef UserSettingId IdType;
      static IdType invalidId() { return IdType(); }
      static const char *surrogateIdField() { return 0; }
    };
  }
}

class User::Setting
{
public:
  Setting();
  Setting(const dbo::ptr<User> &user, const std::string &name, const std::string &value);
  typedef UserSettingId Id;
  template<class Action>
  void persist(Action& a);

  template<typename T> static T value(dbo::Transaction &transaction, const std::string &key, const dbo::ptr<User> &user, const T &defaultValue = T()) {
    auto found = find(key, user, transaction);
    return found ? boost::lexical_cast<T>(found->_value) : defaultValue;
  }

  template<typename T> static void setValue(dbo::Transaction &transaction, const std::string &key, const dbo::ptr<User> &user, const T &value) {
    saveOrUpdate(key, boost::lexical_cast<std::string>(value), user, transaction);
  }

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
  }
}


template<typename Action> void User::Setting::persist(Action& a)
{
  dbo::id(a, _id);
  dbo::field(a, _value, "value");
}

#endif // USERSETTING_H
