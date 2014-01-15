#include "usersettingspage.h"
#include "private/usersettingspage_p.h"

#include <utils/d_ptr_implementation.h>
#include "session.h"
#include <Wt/WContainerWidget>
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WGroupBox>
#include <Wt/WLabel>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/Login>
#include <Wt/Auth/AbstractUserDatabase>
#include "astroplanner.h"
#include <Wt/Auth/PasswordStrengthValidator>
#include "Models"

using namespace Wt;
using namespace WtCommons;
using namespace std;

UserSettingsPage::Private::Private(Session &session, UserSettingsPage *q): session(session), q(q)
{
}

UserSettingsPage::UserSettingsPage(Session &session, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), d(session, this)
{
  WContainerWidget *content = WW<WContainerWidget>();
  setImplementation(content);
  WGroupBox *changePassword = WW<WGroupBox>(WString::tr("user_settings_change_password"), content);

  auto controlElement = [=] (WFormWidget *w, const string &labelKey = string() ) {
    WContainerWidget *container = WW<WContainerWidget>().css("control-group");
    if(!labelKey.empty()) {
      WLabel *label = WW<WLabel>(WString::tr(labelKey), container).css("control-label");
      label->setBuddy(w);
    }
    container->addWidget(WW<WContainerWidget>().css("controls").add(w));
    return container;
  };

  WLineEdit *oldPassword = WW<WLineEdit>();
  WLineEdit *newPassword = WW<WLineEdit>();
  WLineEdit *newPasswordConfirm = WW<WLineEdit>();
  for(auto edit: vector<WLineEdit*>{oldPassword, newPassword, newPasswordConfirm})
    edit->setEchoMode(WLineEdit::Password);
  WPushButton *changePasswordButton = WW<WPushButton>(WString::tr("user_settings_change_password")).css("btn btn-primary").onClick([=](WMouseEvent) {
    Auth::PasswordService &passwordService = d->session.passwordAuth();
    if(!passwordService.verifyPassword(d->session.login().user(), oldPassword->text() )) {
      AstroPlanner::instance()->notification(WString::tr("changepwd_error_title"), WString::tr("changepwd_wrong_password"), AstroPlanner::Error, 10);
      return;
    }
    if(newPassword->text() != newPasswordConfirm->text()) {
      AstroPlanner::instance()->notification(WString::tr("changepwd_error_title"), WString::tr("changepwd__passwords_not_matching"), AstroPlanner::Error, 10);
      return;
    }
    string email = d->session.login().user().email().empty() ? d->session.login().user().unverifiedEmail() : d->session.login().user().email();
    WValidator::Result passwordValidation = passwordService.strengthValidator()->validate(newPassword->text(), d->session.login().user().identity("loginname"), email);
    if( passwordValidation.state() != WValidator::Valid ) {
      AstroPlanner::instance()->notification(WString::tr("changepwd_error_title"), passwordValidation.message(), AstroPlanner::Error, 10);
      return;
    }
    passwordService.updatePassword(d->session.login().user(), newPassword->text());
    AstroPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("changepwd_passwords_changed"), AstroPlanner::Success, 10);
  });

  changePassword->addWidget(WW<WContainerWidget>().css("form-horizontal")
                            .add(controlElement(oldPassword, "user_settings_old_password"))
                            .add(controlElement(newPassword, "user_settings_new_password"))
                            .add(controlElement(newPasswordConfirm, "user_settings_new_password_confirm"))
                            .add(controlElement(changePasswordButton))
                            );
}

UserSettingsPage::~UserSettingsPage()
{
}

