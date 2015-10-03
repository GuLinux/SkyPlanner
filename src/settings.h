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
#include <boost/optional.hpp>

class Settings
{
public:
    typedef std::shared_ptr<Settings> ptr;
    class Reader {
    public:
      virtual boost::optional<std::string> value(const std::string &key) const = 0;
      typedef std::shared_ptr<Reader> ptr;
    };
    
    template<typename T> class optional {
    public:
      optional(const boost::optional<T> &value, const T &default_value) : _value(value), _default_value(default_value) {}
      bool is_set() const { return (_value ? true: false); }
      operator T() const { if(_value) return *_value; return _default_value; };
      T value() const { return *this; }
    private:
      boost::optional<T> _value;
      T _default_value;
    };
    
    Settings(const Reader::ptr &reader);
    ~Settings();
    static Settings &instance();
    
    boost::optional<std::string> google_api_key() const;
    optional<std::string> style_css_path() const;
    boost::optional<std::string> admin_password() const;
    boost::optional<std::string> openweather_api_key() const;
    optional<std::string> show_sql_queries() const;
    optional<std::string> sqlite_database() const;
    boost::optional<std::string> psql_connection() const;
    optional<std::string> admin_name() const;
    optional<std::string> admin_email() const;
    optional<std::string> strings_dir() const;
    boost::optional<std::string> theme_css() const;
    boost::optional<std::string> google_analytics_ua() const;
    boost::optional<std::string> google_analytics_domain() const;
    optional<std::string> dss_cache_path() const;
    boost::optional<std::string> dss_cache_url() const;
private:
  D_PTR;
};

class EnvironmentReader : public Settings::Reader {
public:
  virtual boost::optional<std::string> value(const std::string& key) const;
};

class WServerConfigurationReader : public Settings::Reader {
public:
  virtual boost::optional<std::string> value(const std::string& key) const;
};

class CompositeReader : public Settings::Reader {
public:
  CompositeReader(const std::initializer_list<Reader::ptr> &readers);
  virtual boost::optional<std::string> value(const std::string& key) const;
private:
  D_PTR
};

#endif // SETTINGS_H
