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
#include "Models"
#include "utils/format.h"

using namespace std;
using namespace Wt;


NebulaDenomination::NebulaDenomination(const CataloguePtr &catalogue, boost::optional<string> number, std::string name, boost::optional<string> comment, boost::optional<string> otherCatalogues)
  : _catalogue(catalogue), _number(number), _name(name), _comment(comment), _otherCatalogues(otherCatalogues)
{
}

CataloguePtr NebulaDenomination::catalogue() const
{
  return _catalogue;
}

boost::optional<string> NebulaDenomination::comment() const
{
  return _comment;
}
string NebulaDenomination::name() const
{
  return _name;
}
NgcObjectPtr NebulaDenomination::ngcObject() const
{
  return _ngcObject;
}
boost::optional<std::string> NebulaDenomination::number() const
{
  return _number;
}


bool NebulaDenomination::isNgcIc() const
{
  return catalogue() && _number && (catalogue()->code() == "NGC" || catalogue()->code() == "IC");
}

string NebulaDenomination::search() const {
 switch(catalogue()->searchMode()) {
   case Catalogue::ByName:
      return name();
    case Catalogue::ByCatalog:
      return format("%s %s") % catalogue()->name() % *_number;
    case Catalogue::ByNameAndType:
      return format("%s %s") % _ngcObject->typeDescription().toUTF8() % name();
 }
}

void NebulaDenomination::setComment(const string &comment)
{
  _comment.reset(comment);
}

NebulaDenomination::operator Json::Object()
{
  Json::Object object;
  object["catalogue"] = _catalogue.id();
  object["object"] = _ngcObject.id();
  object["name"] = {_name};
  object["number"] = {_number ? *_number : ""};
  object["comment"] = {_comment ? *_comment : ""};
  object["other-catalogues"] =  {_otherCatalogues ? *_otherCatalogues : ""};
  return object;
}
