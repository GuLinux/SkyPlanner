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

#include "ngcresource.h"
#include "session.h"
#include "ngcobject.h"
#include "nebuladenomination.h"
#include <Wt/Http/Response>
#include <map>
using namespace std;
using namespace Wt;

NgcResource::NgcResource(WObject* parent)
  : WResource(parent)
{

}
NgcResource::~NgcResource()
{

}

map<string,string> Catalogs {
  {"C", "Caldwell"},
  {"M", "Messier"},
  {"NGC", "NGC"},
  {"IC", "IC"},
};

void NgcResource::handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response)
{
  Session session;
  
  if(request.getParameter("cat") == 0 || request.getParameter("num") == 0) {
    response.out() << "Error! missing 'cat' or 'num' parameter" << endl;
    return;
  }
  string catalogue = Catalogs[*request.getParameter("cat")];
  string number = *request.getParameter("num");
  Dbo::Transaction t(session);
  Dbo::ptr<NebulaDenomination> objectDenomination = session.find<NebulaDenomination>().where("catalogue = ?").where("number = ?").bind(catalogue).bind(number);
  if(!objectDenomination) {
    response.out() << "Object " << catalogue << " " << number << " not found!" << endl;
    return;
  }
  Dbo::ptr<NgcObject> object = objectDenomination->ngcObject();
  response.out() << "Object found: " << objectDenomination->catalogue() << objectDenomination->number() << endl;
  response.out() << "AR: " << object->rightAscension() << ", DEC: " << object->declination() << ", magnitude: " << object->magnitude() << endl;
  for(Dbo::ptr<NebulaDenomination> denomination: object->nebulae()) {
    response.out() << "Denomination: " << denomination->name() << ", " << denomination->comment() << endl;
  }
}
