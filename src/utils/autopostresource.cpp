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

#include "autopostresource.h"
#include <Wt/Http/Response>
using namespace std;
using namespace Wt;

AutoPostResource::AutoPostResource(const string &postUrl, const map<string,string> &params, WObject *parent)
  : WResource(parent), postUrl(postUrl), params(params)
{
}

void AutoPostResource::handleRequest (const Wt::Http::Request &request, Wt::Http::Response &response)
{
  response.out() << "<html><head>";
  response.out() << "</head><body>";
  response.out() << "<form id='autopost' action=\"" << postUrl << "\" method='POST'>";
  for(auto hidden: params) {
    response.out() << "<input type='hidden' name='" << hidden.first << "' value='" << hidden.second << "' />";
  }
  response.out() << "<input type='submit' value=\"Click here if you're not automatically redirected\" />";
  response.out() << "</form>";
  response.out() << "<script type=\"text/javascript\">document.getElementById(\"autopost\").submit();</script>";
  response.out() << "</body></html>";
}