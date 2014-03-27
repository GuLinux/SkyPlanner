/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#include "dssimage.h"
#include "private/dssimage_p.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/WContainerWidget>
#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Http/Client>
#include "utils/format.h"
#include <utils/utils.h>
#include <boost/filesystem.hpp>
#include "skyplanner.h"
#include <Wt-Commons/wt_helpers.h>
#include "utils/curl.h"
#include <boost/thread.hpp>
#include <Wt/WProgressBar>
#include <mutex>


using namespace Wt;
using namespace WtCommons;
using namespace std;
namespace fs = boost::filesystem;


std::map<DSSImage::ImageVersion,std::string> DSSImage::Private::imageVersionStrings{
  { poss2ukstu_red, "poss2ukstu_red" },
  { poss2ukstu_blue, "poss2ukstu_blue" },
  { poss2ukstu_ir, "poss2ukstu_ir" },
  { poss1_red, "poss1_red" },
  { poss1_blue, "poss1_blue" },
  { quickv, "quickv" },
  { phase2_gsc2, "phase2_gsc2" },
  { phase2_gsc1, "phase2_gsc1" },
};


DSSImage::Private::Private( const Coordinates::Equatorial &coordinates, const Angle &size, DSSImage::ImageVersion imageVersion, const shared_ptr<mutex> &downloadMutex, DSSImage *q )
  : coordinates(coordinates), size(size), imageVersion(imageVersion), downloadMutex(downloadMutex), q( q )
{
  string cacheDir("dss-cache");
  wApp->readConfigurationProperty("dss-cache-dir", cacheDir);
  fs::create_directories(cacheDir);
  cacheFile = fs::path(cacheDir) / cacheKey();
}

DSSImage::~DSSImage()
{
  d->aborted = true;
  d->downloadThread.join();
}

Signal<WMouseEvent> &DSSImage::imageClicked() const
{
  return d->imageClicked;
}

DSSImage::ImageVersion DSSImage::imageVersion( const string &version )
{
  for(auto v: Private::imageVersionStrings) {
    if(version == v.second)
      return v.first;
  }
  return phase2_gsc2;
}

string DSSImage::imageVersion( const DSSImage::ImageVersion &version )
{
  return Private::imageVersionStrings[version];
}

vector< DSSImage::ImageVersion > DSSImage::versions()
{
  vector<ImageVersion> v;
  transform(begin(Private::imageVersionStrings), end(Private::imageVersionStrings), back_inserter(v), [](pair<ImageVersion,string> p) { return p.first; });
  return v;
}



string DSSImage::Private::cacheKey() const
{
  return format("%s-ar_%d-%d-%.1f_dec_%d-%d-%.1f_size_%d-%d-%.1f.gif")
  % imageVersionStrings[imageVersion]
  % coordinates.rightAscension.sexagesimalHours().hours
  % coordinates.rightAscension.sexagesimalHours().minutes
  % coordinates.rightAscension.sexagesimalHours().seconds
  % coordinates.declination.sexagesimal().degrees
  % coordinates.declination.sexagesimal().minutes
  % coordinates.declination.sexagesimal().seconds
  % size.sexagesimal().degrees
  % size.sexagesimal().minutes
  % size.sexagesimal().seconds;
}
string DSSImage::Private::imageLink() const
{
  Angle::Sexagesimal objectSize = size.sexagesimal();
  double objectRect = static_cast<double>(objectSize.degrees * 60) + static_cast<double>(objectSize.minutes) + (objectSize.seconds/60);
  double multiplyFactor = 3.0;
  if(objectRect < 20)
    multiplyFactor = 5.0;
  if(objectRect < 10)
    multiplyFactor = 6.5;
  if(objectRect < 5 )
    multiplyFactor = 10.;
  if(objectRect < 1 )
    multiplyFactor = 20.;
  
  objectRect = min(75.0, objectRect * multiplyFactor);
  objectRect = objectRect <= 0 ? 75.0:objectRect; // objects without angular size (-1), showing max possible field...

  return format("http://archive.stsci.edu/cgi-bin/dss_search?v=%s&r=%d+%d+%.1f&d=%d+%d+%.1f&e=J2000&h=%d&w=%df&f=gif&c=none&fov=SM97&v3=")
  % imageVersionStrings[imageVersion]
  % coordinates.rightAscension.sexagesimalHours().hours
  % coordinates.rightAscension.sexagesimalHours().minutes
  % coordinates.rightAscension.sexagesimalHours().seconds
  % coordinates.declination.sexagesimal().degrees
  % coordinates.declination.sexagesimal().minutes
  % coordinates.declination.sexagesimal().seconds
  % objectRect % objectRect;
}

void DSSImage::Private::setCacheImage()
{
  content->clear();
  string deployPath;
  if(wApp->readConfigurationProperty("dsscache_deploy_path", deployPath )) {
    _imageLink.setUrl(format("%s/%s") % deployPath % boost::filesystem::path(cacheFile).filename().string());
  } else
    _imageLink.setResource(new WFileResource(cacheFile.string(), q));
  if(showAnchor) {
    content->addWidget(WW<WAnchor>(_imageLink).setTarget(TargetNewWindow).add(WW<WImage>(_imageLink).addCss("img-responsive")));
  }
  else {
    content->addWidget(WW<WImage>(_imageLink).addCss("img-responsive").onClick([=](const WMouseEvent &e){imageClicked.emit(e); }));
  }
  _loaded.emit(_imageLink);
}

struct CurlProgressHandler {
    WApplication *app;
    WProgressBar *progressBar = nullptr;
    void finished() { delete progressBar; progressBar = nullptr; }
    bool is_finished() const { return progressBar == nullptr; }
    std::mutex mutex;
};

WLink DSSImage::imageLink() const
{
  return d->_imageLink;
}

void DSSImage::Private::curlDownload()
{
    content->clear();
    content->addWidget(new WText(WString::tr("dss_downloading_message")));
    shared_ptr<CurlProgressHandler> progressHandler{new CurlProgressHandler};
    /*progressHandler->progressBar = new WProgressBar();
    progressHandler->progressBar->setMaximum(100);
    */
    content->addWidget(new WBreak);
    content->addWidget(WW<WImage>("http://gulinux.net/loading_animation.gif").addCss("center-block"));

    //content->addWidget(progressHandler->progressBar);
    progressHandler->app = wApp;

    downloadThread = boost::thread([=] () mutable {
      unique_lock<mutex> scheduledDownloadLock;
      if(downloadMutex) {
        WServer::instance()->log("notice") << "sequential download: waiting for mutex....";
        scheduledDownloadLock = unique_lock<mutex>(*downloadMutex);
        WServer::instance()->log("notice") << "sequential download: mutex free, starting";
      }
        ofstream output(cacheFile.string());
        shared_ptr<Curl> curl(new Curl{output});
        curl->setProgressCallback([=](double, double, double percent) {
          return aborted ? 1 : 0;
          /*
          std::unique_lock<std::mutex> lock(progressHandler->mutex);
          if( progressHandler->is_finished() || percent > 99 || ! percent > progressHandler->progressBar->value()) return;
          WServer::instance()->post(progressHandler->app->sessionId(), [=]{
            progressHandler->progressBar->setValue(percent);
            progressHandler->app->triggerUpdate();
          });
            */
        });
        curl->get(imageLink());
        WServer::instance()->post(progressHandler->app->sessionId(), [=] {
            Scope triggerUpdate([=]{ progressHandler->app->triggerUpdate(); });
            std::unique_lock<std::mutex> lock(progressHandler->mutex);
            progressHandler->finished();
            if( ! curl->requestOk() || curl->httpResponseCode() != 200 || curl->contentType() != "image/gif" ) {
                WServer::instance()->log("warning") << "Error downloading data using libCURL: " << curl->lastErrorMessage();
                boost::filesystem::remove(cacheFile);
                content->addWidget(new WText(WString::tr("dss_download_error")));
                failed.emit();
                return;
            }
            setCacheImage();
        });
    });
    downloadThread.detach();
}


Signal<> &DSSImage::failed() const {
  return d->failed;
}

Signal<WLink> &DSSImage::imageLoaded() const {
  return d->_loaded;
}

DSSImage::DSSImage(const Coordinates::Equatorial &coordinates, const Angle &size, DSSImage::ImageVersion imageVersion, const shared_ptr<mutex> &downloadMutex, bool anchor, bool showDSSLink, WContainerWidget *parent )
  : WCompositeWidget(parent), d( coordinates, size, imageVersion, downloadMutex, this )
{
  d->content = new WContainerWidget;
  WContainerWidget *container = WW<WContainerWidget>();
  if(showDSSLink) {
    WAnchor *original = new WAnchor(d->imageLink(), "Original DSS Image Link");
    original->setInline(false);
    original->setTarget(Wt::TargetNewWindow);
    container->addWidget(original);
  }
  container->addWidget(d->content);
  setImplementation(container);
  d->showAnchor = anchor;
  if(fs::exists(d->cacheFile)) {
    d->setCacheImage();
  }
  else {
    d->curlDownload();
  }
}

WLink DSSImage::dssOriginalLink() const
{
  return d->imageLink();
}

void DSSImage::startDownload()
{
  d->curlDownload();
}

