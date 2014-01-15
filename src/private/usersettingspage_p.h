#ifndef USERSETTINGSPAGE_P_H
#define USERSETTINGSPAGE_P_H

#include "usersettingspage.h"

class UserSettingsPage::Private
{
public:
  Private(Session &session, UserSettingsPage *q);
  Session &session;
private:
  UserSettingsPage *q;
};

#endif // USERSETTINGSPAGE_H


