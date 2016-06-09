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
#include "usersettingspage.h"
#include "private/usersettingspage_p.h"

#include "session.h"
#include <Wt/WContainerWidget>
#include "wt_helpers.h"
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
#include "wform.h"


using namespace Wt;
using namespace WtCommons;
using namespace std;

UserSettingsPage::Private::Private(Session &session, UserSettingsPage *q): session(session), q(q)
{
}

UserSettingsPage::UserSettingsPage(Session &session, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), dptr(session, this)
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
  Dbo::Transaction t(session);
  WGroupBox *changePassword = WW<WGroupBox>(WString::tr("user_settings_change_password"), content);
  bool hasOldPassword = ! session.login().user().password().empty();


  WLineEdit *oldPassword = WW<WLineEdit>();
  WLineEdit *newPassword = WW<WLineEdit>();
  WLineEdit *newPasswordConfirm = WW<WLineEdit>();
  for(auto edit: vector<WLineEdit*>{oldPassword, newPassword, newPasswordConfirm})
    edit->setEchoMode(WLineEdit::Password);
  WPushButton *changePasswordButton = WW<WPushButton>(WString::tr("user_settings_change_password")).css("btn btn-primary").onClick([=](WMouseEvent) {
    Dbo::Transaction t(session);
    Auth::PasswordService &passwordService = session.passwordAuth();
    if(hasOldPassword && ! passwordService.verifyPassword(session.login().user(), oldPassword->text() )) {
      SkyPlanner::instance()->notification(WString::tr("changepwd_error_title"), WString::tr("changepwd_wrong_password"), Notification::Error, 10);
      return;
    }
    if(newPassword->text() != newPasswordConfirm->text()) {
      SkyPlanner::instance()->notification(WString::tr("changepwd_error_title"), WString::tr("changepwd__passwords_not_matching"), Notification::Error, 10);
      return;
    }
    string email = session.login().user().email().empty() ? session.login().user().unverifiedEmail() : session.login().user().email();
    WValidator::Result passwordValidation = passwordService.strengthValidator()->validate(newPassword->text(), session.user()->loginName(), email);
    if( passwordValidation.state() != WValidator::Valid ) {
      SkyPlanner::instance()->notification(WString::tr("changepwd_error_title"), passwordValidation.message(), Notification::Error, 10);
      return;
    }
    passwordService.updatePassword(session.login().user(), newPassword->text());
    oldPassword->setText(WString());
    newPassword->setText(WString());
    newPasswordConfirm->setText(WString());
    SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("changepwd_passwords_changed"), Notification::Success, 10);
  });

  auto enableChangePasswordButton = [=] {
    bool enable = true;
    for(auto edit: vector<WLineEdit*>{newPassword, newPasswordConfirm})
      enable &= !edit->text().empty();
    if(hasOldPassword) {
      enable &= !oldPassword->text().empty();
      enable &= oldPassword->text() != newPassword->text();
    }
    enable &= newPassword->text() == newPasswordConfirm->text();
    changePasswordButton->setEnabled(enable);
  };

  oldPassword->keyWentUp().connect([=](WKeyEvent) { enableChangePasswordButton(); });
  newPassword->keyWentUp().connect([=](WKeyEvent) { enableChangePasswordButton(); });
  newPasswordConfirm->keyWentUp().connect([=](WKeyEvent) { enableChangePasswordButton(); });
  enableChangePasswordButton();

  changePassword->addWidget(WW<WForm>(WForm::Horizontal, 5).addCss("col-sm-6").get()
                            ->add(oldPassword, "user_settings_old_password")
                            ->add(newPassword, "user_settings_new_password")
                            ->add(newPasswordConfirm, "user_settings_new_password_confirm")
                            ->addButton(changePasswordButton)
                            );

  oldPassword->setHidden(!hasOldPassword);

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
    SkyPlanner::instance()->notification(WString::tr("user_settings_new_email"), WString::tr("user_settings_email_changed_notify") + WString::tr("user_settings_email_verification_notify"), Notification::Information, 10);
  });


  newEmail->keyWentUp().connect([=](WKeyEvent) { enableChangeButton(); });

  WToolBar *toolbar = WW<WToolBar>().addButton(changeAddress);
  if(needsVerification) {
    WPushButton *resendVerification = WW<WPushButton>(WString::tr("user_settings_resend_verification")).css("btn");
    changeAddress->clicked().connect([=](WMouseEvent) { resendVerification->disable(); });
    resendVerification->clicked().connect([=](WMouseEvent) {
      session.auth().verifyEmailAddress(user, user.unverifiedEmail());
      resendVerification->disable();
      SkyPlanner::instance()->notification(WString::tr("user_settings_email_verification_notify_title"), WString::tr("user_settings_email_verification_notify"), Notification::Information, 10);
    });
    toolbar->addButton(resendVerification);
  }
  email->addWidget(WW<WForm>(WForm::Horizontal).addCss("col-sm-6").get()
                   ->add(currentEmail, "user_settings_current_email")
                   ->add(newEmail, "user_settings_new_email")
                   ->addButton(toolbar)
                   );
  enableChangeButton();
}

UserSettingsPage::~UserSettingsPage()
{
}

