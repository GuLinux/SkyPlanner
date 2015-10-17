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

#ifndef SETTINGS_H
#define SETTINGS_H
#include "c++/dptr.h"
#include "c++/settings.h"
#include <boost/optional.hpp>

class Settings
{
public:
    typedef std::shared_ptr<Settings> ptr;
   
    Settings(const GuLinux::Settings::Reader::ptr &reader);
    ~Settings();
    static Settings &instance();
    
    boost::optional<std::string> google_api_key() const;
    GuLinux::Settings::value_with_default<std::string> style_css_path() const;
    boost::optional<std::string> admin_password() const;
    boost::optional<std::string> openweather_api_key() const;
    GuLinux::Settings::value_with_default<std::string> show_sql_queries() const;
    GuLinux::Settings::value_with_default<std::string> sqlite_database() const;
    boost::optional<std::string> psql_connection() const;
    GuLinux::Settings::value_with_default<std::string> admin_name() const;
    GuLinux::Settings::value_with_default<std::string> admin_email() const;
    GuLinux::Settings::value_with_default<std::string> strings_dir() const;
    boost::optional<std::string> theme_css() const;
    boost::optional<std::string> google_analytics_ua() const;
    boost::optional<std::string> google_analytics_domain() const;
    GuLinux::Settings::value_with_default<std::string> resources_path() const;
    GuLinux::Settings::value_with_default<std::string> dss_cache_path() const;
    boost::optional<std::string> dss_cache_url() const;
private:
  D_PTR;
};


#endif // SETTINGS_H
