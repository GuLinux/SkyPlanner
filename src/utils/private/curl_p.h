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

#ifndef CURL_P_H
#define CURL_P_H
#include "utils/curl.h"
#include <curl/curl.h>
#include <sstream>

class Curl::Private
{
  public:
    Private( std::ostream &output, Curl *q );
    CURL *curl_handle;
    std::ostream &output;
    std::stringstream headers;
    static size_t WriteHeadersCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static size_t WriteToFileCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static int curlProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    ProgressFunc progress = [](double, double, double) { return 0; };
    CURLcode res;
    long responseCode = 0;
    std::map<std::string,std::string> parsedHeaders;
  private:
    class Curl *const q;
};
#endif // CURL_P_H
