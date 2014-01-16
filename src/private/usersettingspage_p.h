#ifndef USERSETTINGSPAGE_P_H
#define USERSETTINGSPAGE_P_H

#include "usersettingspage.h"

class UserSettingsPage::Private
{
public:
  Private(Session &session, UserSettingsPage *q);
  Session &session;
  Wt::WContainerWidget *content;
  void onDisplay();
private:
  UserSettingsPage *q;
};

#endif // USERSETTINGSPAGE_H


