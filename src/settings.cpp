/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
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

#include "settings.h"
#include <list>

using namespace std;

class Settings::Private {
public:
  Private(const Reader::ptr &reader, Settings *q);
  Reader::ptr reader;
private:
  Settings *q;
};

Settings::Private::Private(const Reader::ptr &reader, Settings* q) : reader{reader}, q{q}
{
}

Settings::Settings(const Reader::ptr& reader) : dptr(reader, this)
{
}


Settings::~Settings()
{
}

Settings& Settings::instance()
{
  static Settings settings{make_shared<CompositeReader>( std::initializer_list<Reader::ptr>{make_shared<EnvironmentReader>(), make_shared<WServerConfigurationReader>() })};
  return settings;
}


boost::optional< string > Settings::google_api_key() const
{
  return d->reader->value("google_api_server_key");
}

Settings::optional_value< string > Settings::style_css_path() const
{
  return {d->reader->value("style-css-path"), "/skyplanner_style.css"};
}


class CompositeReader::Private {
public:
  Private(const initializer_list< Reader::ptr >& readers);
  list<Reader::ptr> readers;
};

CompositeReader::Private::Private(const initializer_list< Reader::ptr >& readers) : readers{readers}
{

}

CompositeReader::CompositeReader(const initializer_list< Reader::ptr >& readers) : dptr(readers)
{

}

boost::optional< string > CompositeReader::value(const string& key) const
{
  for(auto reader: d->readers) {
    auto value = reader->value(key);
    if(value)
      return value;
  }
  return {};
}

#include <cstdlib>
boost::optional< std::string > EnvironmentReader::value(const string& key) const
{
  if(getenv(key.c_str()))
    return string{getenv(key.c_str())};
  return {};
}

#include <Wt/WServer>
boost::optional< string > WServerConfigurationReader::value(const string& key) const
{
  string value;
  if(Wt::WServer::instance()->readConfigurationProperty(key, value))
    return value;
  return {};
}


