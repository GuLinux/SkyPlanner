/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Marco Gulino <email>
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

#include "curl.h"
#include "private/curl_p.h"
#include "utils/d_ptr_implementation.h"
#include <iostream>

Curl::Private::Private( std::ostream &output, Curl *q ) : output(output), q( q )
{
}

Curl::Curl( std::ostream &output )
  : d( output, this )
{
  d->curl_handle = curl_easy_init();
  curl_easy_setopt(d->curl_handle, CURLOPT_WRITEFUNCTION, Private::WriteToFileCallback);
  curl_easy_setopt(d->curl_handle, CURLOPT_HEADERFUNCTION, Private::WriteHeadersCallback);
  curl_easy_setopt(d->curl_handle, CURLOPT_PROGRESSFUNCTION, Private::curlProgress);
  curl_easy_setopt(d->curl_handle, CURLOPT_PROGRESSDATA, d.get());
  curl_easy_setopt(d->curl_handle, CURLOPT_NOPROGRESS, 0);

  curl_easy_setopt(d->curl_handle, CURLOPT_WRITEDATA, static_cast<void *>(d.get()));
  curl_easy_setopt(d->curl_handle, CURLOPT_WRITEHEADER, static_cast<void *>(d.get()));
  curl_easy_setopt(d->curl_handle, CURLOPT_ERRORBUFFER, static_cast<void*>(d->errorBuffer));
}

Curl::~Curl()
{
  curl_easy_cleanup(d->curl_handle);
}

#include <Wt/WApplication>
Curl &Curl::get( const std::string &url )
{
  curl_easy_setopt(d->curl_handle, CURLOPT_URL, url.data());
  d->res = curl_easy_perform(d->curl_handle);
  wApp->log("notice") << __PRETTY_FUNCTION__ << "Error: " << d->res << " = " << d->errorBuffer;
  return *this;
}

Curl &Curl::setProgressCallback( Curl::ProgressFunc progressFunc )
{
  d->progress = progressFunc;
  return *this;
}

bool Curl::requestOk()
{
  return d->res == CURLE_OK;
}


size_t Curl::Private::WriteToFileCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    Curl::Private *d = static_cast<Curl::Private*>(userp);
    d->output.write(static_cast<char*>(contents), realsize);
    return realsize;
}
size_t Curl::Private::WriteHeadersCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    Curl::Private *d = static_cast<Curl::Private*>(userp);

    for(int i=0; i<realsize; i++)
        d->headers.push_back(static_cast<char*>(contents)[i]);
    return realsize;
}


int Curl::Private::curlProgress(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if(dltotal <= 0 ) return 0;
    Curl::Private *d = static_cast<Curl::Private*>(clientp);
    double currentPercent = dlnow*100./dltotal;
    d->progress(dlnow, dltotal, currentPercent);
    return 0;
}
