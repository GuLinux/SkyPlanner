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
  Private(Session *q);
  std::shared_ptr<Wt::Dbo::SqlConnection> connection;
  UserDatabase *users;
  Wt::Auth::Login login;
  void init(const std::string &connection, Provider provider);
private:
  Session *q;
};


namespace {
  Wt::Auth::AuthService myAuthService;
  Wt::Auth::PasswordService myPasswordService(myAuthService);
  std::vector<const Wt::Auth::OAuthService*> myOAuthServices;

}
#endif

