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
#include "session.h"
#include "private/session_p.h"
#include <Wt/Dbo/backend/Sqlite3>
#ifdef HAVE_WT_POSTGRES
#include <Wt/Dbo/backend/Postgres>
#else
#warning "Wt Dbo Postgres backend missing!"
#endif
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordVerifier>
#include <Wt/Auth/PasswordStrengthValidator>
#include <Wt/Auth/HashFunction>
#include <Wt/Auth/OAuthService>
#include <Wt/Auth/GoogleService>
#include <Wt/Auth/FacebookService>
#include <Wt/WServer>
#include "ngcobject.h"
#include "nebuladenomination.h"
#include "utils/d_ptr_implementation.h"

using namespace std;
using namespace Wt;

Session::Private::Private(Session *q) : q(q) {
}

void Session::Private::init(const string &connectionString, Session::Provider provider)
{
#ifdef HAVE_WT_POSTGRES
  if(provider == Session::Postgres) {
    connection = make_shared<Dbo::backend::Postgres>(connectionString);
  } else {
    connection = make_shared<Dbo::backend::Sqlite3>(connectionString);
  }
#else
  if(provider == Session::Postgres)
    throw runtime_error("Error! SkyPlanner was compiled without postgresql support");
  connection = make_shared<Dbo::backend::Sqlite3>(connectionString);
#endif
  q->setConnection(*connection);
  string show_queries = "false";
  WServer::instance()->readConfigurationProperty("show-queries", show_queries);
  connection->setProperty("show-queries", show_queries);
  q->mapClass<Catalogue>("catalogues");
  q->mapClass<NgcObject>("objects");
  q->mapClass<NebulaDenomination>("denominations");
  q->mapClass<User>("user");
  q->mapClass<User::Setting>("user_settings");
  q->mapClass<Telescope>("telescope");
  q->mapClass<Eyepiece>("eyepiece");
  q->mapClass<FocalModifier>("focal_modifier");
  q->mapClass<AuthInfo>("auth_info");
  q->mapClass<AuthInfo::AuthIdentityType>("auth_identity");
  q->mapClass<AuthInfo::AuthTokenType>("auth_token");
  q->mapClass<AstroSession>("astro_session");
  q->mapClass<AstroSessionObject>("astro_session_object");
  q->mapClass<EphemerisCache>("ephemeris_cache");
  q->mapClass<ViewPort>("objects_viewport");
  users = new UserDatabase(*q);
  static bool creationScriptPrinted = false;
  static bool createTablesExecuted = false;
  if(!creationScriptPrinted) {
    cerr << "Tables creation script: " << endl;
    cerr << "-----------------------------------------------" << endl;
    cerr << q->tableCreationSql() << endl;
    cerr << "-----------------------------------------------" << endl;
    creationScriptPrinted = true;
  }
  if(!createTablesExecuted) {
    createTablesExecuted = true;
    try {
      q->createTables();
    } catch(Dbo::Exception &e) {
      cerr << "Creation script failed, perhaps schema is already existing?" << endl;
      cerr << "Error details: " << e.what() << endl;
    }
  }
}

Session::Session() : d(this)
{
  Provider provider = Sqlite3;
  string connectionString = "SkyPlanner.sqlite";
#ifdef HAVE_WT_POSTGRES
  if(WServer::instance()->readConfigurationProperty("psql-connection", connectionString))
    provider = Postgres;
#endif
  d->init(connectionString, provider);
}

Session::Session(const string &connection, Provider provider) : d(this)
{
  d->init(connection, provider);
}

const std::vector<const Auth::OAuthService*> &Session::oAuth()
{
  return myOAuthServices;
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
  dbo::ptr<AuthInfo> _authInfo = authInfo();
  dbo::ptr<User> user = _authInfo->user();
  
  if(!user) {
    user = add(new User);
    _authInfo.modify()->setUser(user);
    _authInfo.flush();
  }    
  return _authInfo->user();
}

Dbo::ptr<AuthInfo> Session::authInfo()
{
  return d->users->find(d->login.user());
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
  if (Wt::Auth::GoogleService::configured())
    myOAuthServices.push_back(new Wt::Auth::GoogleService(myAuthService));

  if (Wt::Auth::FacebookService::configured())
    myOAuthServices.push_back(new Wt::Auth::FacebookService(myAuthService));
}

Auth::AuthService& Session::auth()
{
  return myAuthService;
}

Auth::PasswordService& Session::passwordAuth()
{
  return myPasswordService;
}
