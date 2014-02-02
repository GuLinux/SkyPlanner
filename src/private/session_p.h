#ifndef SESSION_P_H
#define SESSION_P_H
#include "session.h"
#include "Models"
#include <Wt/Auth/Login>
#include <Wt/Auth/Dbo/UserDatabase>
#include <Wt/Auth/AuthService>
#include <Wt/Auth/PasswordService>
#include <Wt/Auth/OAuthService>

typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session::Private {
public:
  Private();
  std::shared_ptr<Wt::Dbo::SqlConnection> connection;
  UserDatabase *users;
  Wt::Auth::Login login;
};


namespace {
  Wt::Auth::AuthService myAuthService;
  Wt::Auth::PasswordService myPasswordService(myAuthService);
  std::vector<const Wt::Auth::OAuthService*> myOAuthServices;

}
#endif

