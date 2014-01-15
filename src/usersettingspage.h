#ifndef USERSETTINGSPAGE_H
#define USERSETTINGSPAGE_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
class Session;

class UserSettingsPage : public Wt::WCompositeWidget {
public:
  explicit UserSettingsPage(Session &session, Wt::WContainerWidget *parent = 0);
  ~UserSettingsPage();
private:
  D_PTR;
};

#endif // USERSETTINGSPAGE_H


