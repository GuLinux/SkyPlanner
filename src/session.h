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

#ifndef SESSION_H
#define SESSION_H

#include <Wt/Dbo/Session>
#include "utils/d_ptr.h"

class User;
namespace Wt
{
  namespace Auth
  {
    class Login;
    class AbstractUserDatabase;
    class PasswordService;
    class AuthService;
    namespace Dbo {
      template<typename T> class AuthInfo;
    }
    class OAuthService;
  }
}

class Session : public Wt::Dbo::Session
{
public:
    Session();
    ~Session();
    Wt::Auth::AbstractUserDatabase& users();
    Wt::Auth::Login& login();
    Wt::Dbo::ptr<User> user();
    Wt::Dbo::ptr<Wt::Auth::Dbo::AuthInfo<User>> authInfo();
    static Wt::Auth::AuthService &auth();
    static Wt::Auth::PasswordService &passwordAuth();
    static void configureAuth();
    static const std::vector<const Wt::Auth::OAuthService*> &oAuth();
private:
  D_PTR;
};

#endif // SESSION_H
