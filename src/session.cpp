/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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

#include "session.h"
#include "private/session_p.h"
#include <Wt/Dbo/backend/Sqlite3>
#include <Wt/Dbo/backend/Postgres>
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Auth/HashFunction>
#include <Wt/WServer>
#include "ngcobject.h"
#include "nebuladenomination.h"
#include "utils/d_ptr_implementation.h"

using namespace std;
using namespace Wt;

Session::Private::Private() {
}


Session::Session()
{
  string connectionString;
  if(WServer::instance()->readConfigurationProperty("psql-connection", connectionString)) {
    d->connection = make_shared<Dbo::backend::Postgres>(connectionString);
  } else {
    d->connection = make_shared<Dbo::backend::Sqlite3>("ngc.sqlite");
  }
  setConnection(*d->connection);
  d->connection->setProperty("show-queries", "false");
  mapClass<NgcObject>("objects");
  mapClass<NebulaDenomination>("denominations");
  mapClass<User>("user");
  mapClass<Telescope>("telescope");
  mapClass<AuthInfo>("auth_info");
  mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  mapClass<AuthInfo::AuthTokenType>("auth_token");
  mapClass<AstroSession>("astro_session");
  mapClass<AstroSessionObject>("astro_session_object");
  d->users = new UserDatabase(*this);
  static bool creationScriptPrinted = false;
  if(!creationScriptPrinted) {
    cerr << "Tables creation script: " << endl;
    cerr << "-----------------------------------------------" << endl;
    cerr << tableCreationSql() << endl;
    cerr << "-----------------------------------------------" << endl;
    creationScriptPrinted = true;
  }
  try {
    createTables();
  } catch(Dbo::Exception &) {
    cerr << "Creation script failed, perhaps schema is already existing?" << endl;
  }
}

Session::~Session()
{
}

Auth::AbstractUserDatabase& Session::users()
{
  return *d->users;
}

Auth::Login& Session::login()
{
  return d->login;
}

Dbo::ptr<User> Session::user()
{
  if (!d->login.loggedIn())
    return dbo::ptr<User>();
  dbo::ptr<AuthInfo> authInfo = d->users->find(d->login.user());
  dbo::ptr<User> user = authInfo->user();
  
  if(!user) {
    user = add(new User);
    authInfo.modify()->setUser(user);
    authInfo.flush();
  }    
  return authInfo->user();
}

void Session::configureAuth()
{
  myAuthService.setAuthTokensEnabled(true, "logincookie");
  myAuthService.setEmailVerificationEnabled(true);

  Wt::Auth::PasswordVerifier *verifier = new Wt::Auth::PasswordVerifier();
  verifier->addHashFunction(new Wt::Auth::BCryptHashFunction(7));
  myPasswordService.setVerifier(verifier);
  myPasswordService.setAttemptThrottlingEnabled(true);
  Auth::PasswordStrengthValidator *passwordValidator = new Auth::PasswordStrengthValidator();
  passwordValidator->setMandatory(true);
  passwordValidator->setMinimumLength(Auth::PasswordStrengthValidator::OneCharClass, 8);
  myPasswordService.setStrengthValidator(passwordValidator);
}

const Auth::AuthService& Session::auth()
{
  return myAuthService;
}

const Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}
