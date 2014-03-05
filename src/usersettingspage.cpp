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
#include "skyplanner.h"
#include <Wt/Auth/PasswordStrengthValidator>
#include "Models"
#include <Wt/WToolBar>
#include <Wt/Auth/AuthService>


using namespace Wt;
using namespace WtCommons;
using namespace std;

UserSettingsPage::Private::Private(Session &session, UserSettingsPage *q): session(session), q(q)
{
}

UserSettingsPage::UserSettingsPage(Session &session, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), d(session, this)
{
  d->content = WW<WContainerWidget>();
  setImplementation(d->content);
  auto setupPage = [this,&session] {
    if(!wApp->internalPathMatches("/settings") || !session.user() ) {
      return;
    }
    d->onDisplay();
  };

  wApp->internalPathChanged().connect([this,setupPage,&session](const string &newPath, ...) {
    setupPage();
  });
  setupPage();
}

void UserSettingsPage::Private::onDisplay()
{
  content->clear();
  WGroupBox *changePassword = WW<WGroupBox>(WString::tr("user_settings_change_password"), content);
#define labelSize "5"
#define controlSize "7"

  auto controlElement = [=] (WWidget *w, const string &labelKey = string() ) {
    WContainerWidget *container = WW<WContainerWidget>().css("form-group");
    if(!labelKey.empty()) {
      WLabel *label = WW<WLabel>(WString::tr(labelKey), container).css("control-label col-sm-" labelSize);
      w->addStyleClass("form-control");
      if(dynamic_cast<WFormWidget*>(w))
        label->setBuddy(dynamic_cast<WFormWidget*>(w));
    }
    container->addWidget(WW<WContainerWidget>().css(labelKey.empty() ? "col-sm-" controlSize " col-sm-offset-" labelSize : "col-sm-" controlSize).add(w));
    return container;
  };

  WLineEdit *oldPassword = WW<WLineEdit>();
  WLineEdit *newPassword = WW<WLineEdit>();
  WLineEdit *newPasswordConfirm = WW<WLineEdit>();
  for(auto edit: vector<WLineEdit*>{oldPassword, newPassword, newPasswordConfirm})
    edit->setEchoMode(WLineEdit::Password);
  WPushButton *changePasswordButton = WW<WPushButton>(WString::tr("user_settings_change_password")).css("btn btn-primary").onClick([=](WMouseEvent) {
    Auth::PasswordService &passwordService = session.passwordAuth();
    if(!passwordService.verifyPassword(session.login().user(), oldPassword->text() )) {
      SkyPlanner::instance()->notification(WString::tr("changepwd_error_title"), WString::tr("changepwd_wrong_password"), SkyPlanner::Notification::Error, 10);
      return;
    }
    if(newPassword->text() != newPasswordConfirm->text()) {
      SkyPlanner::instance()->notification(WString::tr("changepwd_error_title"), WString::tr("changepwd__passwords_not_matching"), SkyPlanner::Notification::Error, 10);
      return;
    }
    string email = session.login().user().email().empty() ? session.login().user().unverifiedEmail() : session.login().user().email();
    WValidator::Result passwordValidation = passwordService.strengthValidator()->validate(newPassword->text(), session.login().user().identity("loginname"), email);
    if( passwordValidation.state() != WValidator::Valid ) {
      SkyPlanner::instance()->notification(WString::tr("changepwd_error_title"), passwordValidation.message(), SkyPlanner::Notification::Error, 10);
      return;
    }
    passwordService.updatePassword(session.login().user(), newPassword->text());
    oldPassword->setText(WString());
    newPassword->setText(WString());
    newPasswordConfirm->setText(WString());
    SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("changepwd_passwords_changed"), SkyPlanner::Notification::Success, 10);
  });

  auto enableChangePasswordButton = [=] {
    bool enable = true;
    for(auto edit: vector<WLineEdit*>{oldPassword, newPassword, newPasswordConfirm})
      enable &= !edit->text().empty();
    enable &= oldPassword->text() != newPassword->text();
    enable &= newPassword->text() == newPasswordConfirm->text();
    changePasswordButton->setEnabled(enable);
  };

  oldPassword->keyWentUp().connect([=](WKeyEvent) { enableChangePasswordButton(); });
  newPassword->keyWentUp().connect([=](WKeyEvent) { enableChangePasswordButton(); });
  newPasswordConfirm->keyWentUp().connect([=](WKeyEvent) { enableChangePasswordButton(); });
  enableChangePasswordButton();

  changePassword->addWidget(WW<WContainerWidget>().css("form-horizontal col-sm-7")
                            .setAttribute("role", "form")
                            .add(controlElement(oldPassword, "user_settings_old_password"))
                            .add(controlElement(newPassword, "user_settings_new_password"))
                            .add(controlElement(newPasswordConfirm, "user_settings_new_password_confirm"))
                            .add(controlElement(changePasswordButton))
                            );


  WGroupBox *email = WW<WGroupBox>(WString::tr("user_settings_email"), content);
  Auth::User user = session.login().user();
  bool mailIsVerified = !user.email().empty();
  bool needsVerification = !user.unverifiedEmail().empty();
  WLineEdit *currentEmail = WW<WLineEdit>(mailIsVerified ? user.email() : user.unverifiedEmail() ).setEnabled(false);
  WLineEdit *newEmail = WW<WLineEdit>(mailIsVerified ? user.email() : user.unverifiedEmail());
  WPushButton *changeAddress = WW<WPushButton>(WString::tr("user_settings_update_email")).css("btn btn-primary");


  auto enableChangeButton = [=] {
    string currentEmail = mailIsVerified ? user.email() : user.unverifiedEmail();
    string newEmailAddress = newEmail->text().toUTF8();
    changeAddress->setEnabled(!newEmailAddress.empty() && newEmailAddress != currentEmail);
  };

  changeAddress->clicked().connect([=](WMouseEvent) {
    user.setEmail({});
    user.setUnverifiedEmail(newEmail->text().toUTF8());
    currentEmail->setText(newEmail->text());
    session.auth().verifyEmailAddress(user, newEmail->text().toUTF8());
    changeAddress->disable();
    SkyPlanner::instance()->notification(WString::tr("user_settings_new_email"), WString::tr("user_settings_email_changed_notify") + WString::tr("user_settings_email_verification_notify"), SkyPlanner::Notification::Information, 10);
  });


  newEmail->keyWentUp().connect([=](WKeyEvent) { enableChangeButton(); });

  WToolBar *toolbar = WW<WToolBar>().addButton(changeAddress);
  if(needsVerification) {
    WPushButton *resendVerification = WW<WPushButton>(WString::tr("user_settings_resend_verification")).css("btn");
    changeAddress->clicked().connect([=](WMouseEvent) { resendVerification->disable(); });
    resendVerification->clicked().connect([=](WMouseEvent) {
      session.auth().verifyEmailAddress(user, user.unverifiedEmail());
      resendVerification->disable();
      SkyPlanner::instance()->notification(WString::tr("user_settings_email_verification_notify_title"), WString::tr("user_settings_email_verification_notify"), SkyPlanner::Notification::Information, 10);
    });
    toolbar->addButton(resendVerification);
  }
  email->addWidget(WW<WContainerWidget>().css("form-horizontal col-sm-7")
                   .setAttribute("role", "form")
                   .add(controlElement(currentEmail, "user_settings_current_email"))
                   .add(controlElement(newEmail, "user_settings_new_email"))
                   .add(controlElement(toolbar))
                   );
  enableChangeButton();
}

UserSettingsPage::~UserSettingsPage()
{
}

