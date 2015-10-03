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
#include <boost/filesystem.hpp>

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

Settings::optional< string > Settings::style_css_path() const
{
  return {d->reader->value("style-css-path"), "/skyplanner_style.css"};
}

boost::optional< string > Settings::admin_password() const
{
  return d->reader->value("quit-password");
}

boost::optional< string > Settings::openweather_api_key() const
{
  return d->reader->value("openweather_api_key");
}

Settings::optional< string > Settings::show_sql_queries() const
{
  return {d->reader->value("show-sql-queries"), "false"};
}

boost::optional< string > Settings::psql_connection() const
{
  return d->reader->value("psql-connection");
}

Settings::optional< string > Settings::sqlite_database() const
{
  return {d->reader->value("sqlite-database"), "SkyPlanner.db"};
}

Settings::optional<string> Settings::admin_name() const
{
  return {d->reader->value("admin-name"), "SkyPlanner Administrator"};
}


Settings::optional<string> Settings::admin_email() const
{
  return {d->reader->value("admin-email"), "root@localhost"};
}


Settings::optional< string > Settings::resources_path() const
{
  return {d->reader->value("resources_path"), (boost::filesystem::path(SHARED_PREFIX) / "strings").string()};
}

Settings::optional< string > Settings::strings_dir() const
{
  return {d->reader->value("strings_directory"), (boost::filesystem::path(SHARED_PREFIX) / "strings").string()};
}

boost::optional< string > Settings::theme_css() const
{
  return d->reader->value("theme-css-path");
}


boost::optional< string > Settings::google_analytics_domain() const
{
  return d->reader->value("google-analytics-domain");
}

boost::optional< string > Settings::google_analytics_ua() const
{
  return d->reader->value("google-analytics-ua");
}

Settings::optional< string > Settings::dss_cache_path() const
{
  return {d->reader->value("dss-cache-dir"), (boost::filesystem::path(DATA_DIR) / "cache" / "SkyPlanner" / "dss").string()};
}



boost::optional< string > Settings::dss_cache_url() const
{
  return d->reader->value("dsscache_deploy_path");
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
#include <boost/algorithm/string.hpp>
boost::optional< std::string > EnvironmentReader::value(const string& key) const
{
  string key_tr = string{"SKYPLANNER_"} + boost::replace_all_copy(key, string{"-"}, string{"_"});
  boost::to_upper(key_tr);
  if(getenv(key_tr.c_str()))
    return string{getenv(key_tr.c_str())};
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


