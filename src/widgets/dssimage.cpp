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
#include <Wt/WSlider>
#include <Wt/WTemplate>
#include <Wt/WDialog>
#include <Wt/WPushButton>
#include <Wt/WSlider>
#include <mutex>
#include <GraphicsMagick/Magick++.h>

#define logD() WServer::instance()->log("notice") << __PRETTY_FUNCTION__ << ": "

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


DSSImage::Private::Private( const DSSImage::ImageOptions &imageOptions, const shared_ptr<mutex> &downloadMutex, DSSImage *q )
  : imageOptions(imageOptions), downloadMutex(downloadMutex), q( q )
{
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

map<DSSImage::ImageSize,DSSImage::Private::Image> DSSImage::Private::imageSizeMap {
    {DSSImage::Full, { 0, "" } },
    {DSSImage::Mid, { 650, "mid_" } },
    {DSSImage::Thumb, { 100, "thumb_" } },
};

boost::filesystem::path DSSImage::Private::Image::file(const DSSImage::ImageOptions &imageOptions)
{
  fs::path p;
  static string cacheDir("dss-cache");
  if(cacheDir.empty()) {
    bool success = wApp->readConfigurationProperty("dss-cache-dir", cacheDir);
    if(!success)
      throw runtime_error("Error reading dss-cache-dir property!");
    fs::create_directories(cacheDir);
  }

  string cacheKey = format("%s%s-ar_%d-%d-%.1f_dec_%d-%d-%.1f_size_%d-%d-%.1f.gif")
  % prefix
  % imageVersionStrings[imageOptions.imageVersion]
  % imageOptions.coordinates.rightAscension.sexagesimalHours().hours
  % imageOptions.coordinates.rightAscension.sexagesimalHours().minutes
  % imageOptions.coordinates.rightAscension.sexagesimalHours().seconds
  % imageOptions.coordinates.declination.sexagesimal().degrees
  % imageOptions.coordinates.declination.sexagesimal().minutes
  % imageOptions.coordinates.declination.sexagesimal().seconds
  % imageOptions.size.sexagesimal().degrees
  % imageOptions.size.sexagesimal().minutes
  % imageOptions.size.sexagesimal().seconds;

  p = fs::path(cacheDir) / cacheKey;
  if( !fs::exists(p)  )
    resize(p, imageOptions);
  return p;
}

void DSSImage::Private::Image::resize(const fs::path &destination, const DSSImage::ImageOptions &imageOptions)
{
  if(pixels == 0) return;
  auto fullImage = imageSizeMap[DSSImage::Full].file(imageOptions).string();
  if(! fs::exists(fullImage)) return;
  Magick::Image image(fullImage);
  image.scale({pixels,pixels});
  image.write(destination.string());
}


string DSSImage::Private::imageLink() const
{
  double objectRect = imageOptions.size.arcMinutes();
  return format("http://archive.stsci.edu/cgi-bin/dss_search?v=%s&r=%d+%d+%.1f&d=%d+%d+%.1f&e=J2000&h=%d&w=%df&f=gif&c=none&fov=SM97&v3=")
  % imageVersionStrings[imageOptions.imageVersion]
  % imageOptions.coordinates.rightAscension.sexagesimalHours().hours
  % imageOptions.coordinates.rightAscension.sexagesimalHours().minutes
  % imageOptions.coordinates.rightAscension.sexagesimalHours().seconds
  % imageOptions.coordinates.declination.sexagesimal().degrees
  % imageOptions.coordinates.declination.sexagesimal().minutes
  % imageOptions.coordinates.declination.sexagesimal().seconds
  % objectRect % objectRect;
}

void DSSImage::Private::setCacheImage()
{
  logD() << "file: " << file();
  content->clear();
  string deployPath;
  _imageLink = linkFor(file());

  if(showAnchor) {
    content->addWidget(WW<WAnchor>(_imageLink).setTarget(TargetNewWindow).add(WW<WImage>(_imageLink).addCss("img-responsive")));
  }
  else {
    content->addWidget(WW<WImage>(_imageLink).addCss("img-responsive").onClick([=](const WMouseEvent &e){imageClicked.emit(e); }));
  }
  _loaded.emit(_imageLink);
}



Wt::WLink DSSImage::Private::linkFor(const boost::filesystem::path &file) const
{
  WLink link;
  string deployPath;
  if(wApp->readConfigurationProperty("dsscache_deploy_path", deployPath )) {
    link.setUrl(format("%s/%s") % deployPath % file.filename().string());
  } else
    link.setResource(new WFileResource(file.string(), q));
  return link;
}


fs::path DSSImage::Private::file(DSSImage::ImageSize imageSize) const
{
  return imageSizeMap[imageSize].file(imageOptions);
}

struct CurlProgressHandler {
    WApplication *app;
    WSlider *progressBar = nullptr;
    void finished() { delete progressBar; progressBar = nullptr; }
    bool is_finished() const { return progressBar == nullptr; }
    std::mutex mutex;
};

WLink DSSImage::fullImageLink() const
{
  if(d->imageOptions.imageSize == Full)
    return d->_imageLink;

  return d->linkFor(d->fullFile());
}

void DSSImage::showImageControls()
{
  d->showImageController();
}

void DSSImage::Private::showImageController()
{
  WDialog *dialog = new WDialog(WString::tr("image_controller_title"));
  dialog->setModal(false);
  dialog->setClosable(true);
  WTemplate *content = new WTemplate(R"(
    <div class="row">
      <table class="col-xs-2">
        <tr><td /><td>${up-button}</td><td /></tr>
        <tr><td>${left-button}</td><td /><td>${right-button}</td></tr>
        <tr><td /><td>${down-button}</td><td /></tr>
      </table>
      <div class="col-xs-8">
        ${tr:imagecontrol-zoom-label}<br />
        ${zoom}<br />
        ${tr:imagecontrol-move-factor-label}<br />
        ${move-factor}<br />
      </div>
    </div>
  )");

  content->addFunction("tr", &WTemplate::Functions::tr);

  WSlider *moveFactor = new WSlider;
  WSlider *zoomLevel = new WSlider;
  moveFactor->setRange(0, 100);
  zoomLevel->setRange(0, 750);
  
  auto moveBy = [=](int ar, int dec) {
    double ratio = static_cast<double>(moveFactor->value()) / 100.;
    double arcMinMove = imageOptions.size.arcMinutes() * ratio;
    imageOptions.coordinates.rightAscension += Angle::arcMinutes(static_cast<double>(ar) * arcMinMove);
    imageOptions.coordinates.declination += Angle::arcMinutes(static_cast<double>(dec) * arcMinMove);
    imageOptions.onViewPortChanged(imageOptions.coordinates, imageOptions.size);
    reload();
  };
  zoomLevel->setValue(imageOptions.size.arcMinutes() * 10);
  moveFactor->setValue(20);
  zoomLevel->valueChanged().connect([=](int, _n5) {
    imageOptions.size = Angle::arcMinutes( static_cast<double>(zoomLevel->value() )/10. ); 
    imageOptions.onViewPortChanged(imageOptions.coordinates, imageOptions.size);
    reload();
  });
  content->bindWidget("move-factor", moveFactor);
  content->bindWidget("zoom", zoomLevel);
  content->bindWidget("up-button", WW<WPushButton>("DEC+").css("btn-sm btn-block").onClick([=](WMouseEvent) { moveBy(0, 1 ); }) );
  content->bindWidget("down-button", WW<WPushButton>("DEC-").css("btn-sm btn-block").onClick([=](WMouseEvent) { moveBy(0, -1. ) ; }) );
  content->bindWidget("left-button", WW<WPushButton>("AR-").css("btn-sm btn-block").onClick([=](WMouseEvent) { moveBy(-1. , 0); }) );
  content->bindWidget("right-button", WW<WPushButton>("AR+").css("btn-sm btn-block").onClick([=](WMouseEvent) { moveBy(1 , 0); }) );
  dialog->contents()->addWidget(content);
  dialog->show();
}

void DSSImage::Private::curlDownload()
{
    content->clear();
    content->addWidget(new WText(WString::tr("dss_downloading_message")));
    shared_ptr<CurlProgressHandler> progressHandler{new CurlProgressHandler};
    /*progressHandler->progressBar = new WSlider();
    progressHandler->progressBar->setMaximum(100);
    */
    content->addWidget(new WBreak);
    content->addWidget(WW<WImage>("http://gulinux.net/loading_animation.gif").addCss("center-block"));

    //content->addWidget(progressHandler->progressBar);
    progressHandler->app = wApp;
    fs::path downloadFile = fullFile();
    logD() << "full file path: " << downloadFile;

    downloadThread = boost::thread([=] () mutable {
      unique_ptr<unique_lock<mutex>> scheduledDownloadLock;
      if(downloadMutex) {
        scheduledDownloadLock.reset(new unique_lock<mutex>(*downloadMutex));
      }
        if(aborted) return;
        ofstream output( downloadFile.string() + "_tmp");
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
//            progressHandler->finished();
            if( ! curl->requestOk() || curl->httpResponseCode() != 200 || curl->contentType() != "image/gif" ) {
                if(aborted) return;
                WServer::instance()->log("warning") << "Error downloading data using libCURL: " << curl->lastErrorMessage();
                boost::filesystem::remove(downloadFile.string() + "_tmp");
                content->addWidget(new WText(WString::tr("dss_download_error")));
                failed.emit();
                return;
            }
            try {
              boost::filesystem::rename(downloadFile.string() + "_tmp", downloadFile.string());
              setCacheImage();
            } catch(std::exception &e) {
              WServer::instance()->log("error") << "Error moving temp download file " << downloadFile.string() + "_tmp to " << downloadFile.string() << ": " << e.what();
            }

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

DSSImage::ImageVersion DSSImage::imageVersion() const
{
  return d->imageOptions.imageVersion;
}

DSSImage::DSSImage(const ImageOptions &imageOptions, const shared_ptr<mutex> &downloadMutex, bool anchor, bool showDSSLink, WContainerWidget *parent )
  : WCompositeWidget(parent), d( imageOptions, downloadMutex, this )
{
  d->container = WW<WContainerWidget>();
  d->showAnchor = anchor;
  d->showDSSLink = showDSSLink;
  setImplementation(d->container);

  d->reload();
}

void DSSImage::Private::reload()
{
  logD();
  container->clear();
  content = new WContainerWidget;
  if(showDSSLink) {
    WAnchor *original = new WAnchor(imageLink(), "Original DSS Image Link");
    original->setInline(false);
    original->setTarget(Wt::TargetNewWindow);
    container->addWidget(original);
  }
  container->addWidget(content);

  if(fs::exists(fullFile() )) {
    logD() << "file exists: setting from cache";
    setCacheImage();
  }
  else {
    logD() << "file not found, downloading";
    curlDownload();
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

