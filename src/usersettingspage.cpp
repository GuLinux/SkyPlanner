#include "usersettingspage.h"
#include "private/usersettingspage_p.h"

#include <utils/d_ptr_implementation.h>
#include <session.h>
#include <Wt/WContainerWidget>
#include "Wt-Commons/wt_helpers.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

UserSettingsPage::Private::Private(Session &session, UserSettingsPage *q): session(session), q(q)
{
}

UserSettingsPage::UserSettingsPage(Session &session, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), d(session, this)
{
}

UserSettingsPage::~UserSettingsPage()
{
}

