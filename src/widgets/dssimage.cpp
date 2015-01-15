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
#include <Magick++.h>
#include "models/Models"
#include <Wt/WDoubleSpinBox>
#include <Wt/WMemoryResource>


using namespace Wt;
using namespace WtCommons;
using namespace std;
namespace fs = boost::filesystem;


DSSImage::Private::Private( const DSSImage::ImageOptions &imageOptions, const shared_ptr<mutex> &downloadMutex, DSSImage *q )
  : imageOptions(imageOptions), downloadMutex(downloadMutex), q( q )
{
  this->imageOptions.size = min(imageOptions.size, Angle::arcMinutes(75));
  httpClient.setTimeout(120);
  httpClient.setMaximumResponseSize(1024*1024*40);
  httpClient.done().connect(bind(&DSSImage::Private::save, this, placeholders::_1, placeholders::_2));
}

DSSImage::~DSSImage()
{
  d->aborted = true;
  d->httpClient.abort();
  d->downloadThread.join();
}


void DSSImage::Private::save(const boost::system::error_code &errorCode, const Http::Message &httpMessage)
{
  shared_ptr<DialogControl::Finish> finishDialogControl(new DialogControl::Finish{dialogControl});
  if(errorCode != boost::system::errc::success) {
    WServer::instance()->log(aborted ? "notice" : "error") << "Download failed for " << imageOptions.url() << ": " << errorCode.message(); 
    if(!aborted) failed.emit();
    return;
  } 
  if(httpMessage.status() != 200) {
    WServer::instance()->log(aborted ? "notice" : "error") << "Wrong http status for " << imageOptions.url() << ": " << httpMessage.status(); 
    if(!aborted) failed.emit();
    return;
  }
  string contentType;
  if(httpMessage.getHeader("Content-Type")) contentType = *httpMessage.getHeader("Content-Type");
  if(contentType != "image/gif") {
    WServer::instance()->log(aborted ? "notice" : "error") << "Wrong content type for " << imageOptions.url() << ": expected image/gif, got " << contentType; 
    if(!aborted) failed.emit();
    return;
  }
  WServer::instance()->log("notice") << imageOptions.url() << " correctly downloaded, saving to " << fullFile();
  try {
    Magick::Blob blob(httpMessage.body().data(), httpMessage.body().size());
    Magick::Image image(blob);
    apply_common_options(image);
    image.write(fullFile().string());
  } catch(std::exception &e) {
    WServer::instance()->log("error") << "Error saving to " << fullFile() << ": " << e.what();
    return;
  }

  if(aborted)
    return;
  
  WServer::instance()->log("notice") << " download successfully finished, calling setImageFromCache";
  setImageFromCache(finishDialogControl);

  wApp->triggerUpdate();
}

Signal<WMouseEvent> &DSSImage::imageClicked() const
{
  return d->imageClicked;
}

map<DSSImage::ImageSize,DSSImage::Private::Image> DSSImage::Private::imageSizeMap {
    {DSSImage::Full, { 0, "" } },
    {DSSImage::Mid, { 700, "mid_" } },
    {DSSImage::Thumb, { 100, "thumb_" } },
};

boost::filesystem::path DSSImage::Private::Image::file(const DSSImage::ImageOptions &imageOptions)
{
  static string cacheDir("dss-cache");
  wApp->readConfigurationProperty("dss-cache-dir", cacheDir);
  if(!fs::create_directories(cacheDir)) {
      throw runtime_error("Error reading dss-cache-dir property!");
  }

  auto p = imageOptions.file(cacheDir, prefix);

  if( !fs::exists(p)  )
    resize(p, imageOptions);
  return p;
}

boost::filesystem::path DSSImage::ImageOptions::file(const boost::filesystem::path& cache_dir, const string &prefix) const
{
  string arSignFix = coordinates.rightAscension.degrees() < 0 ? "-" : "";
  string decSignFix = coordinates.declination.degrees() < 0 ? "-" : "";
  string cacheKey = format("%s%s-ar_%s%d-%d-%.1f_dec_%s%d-%d-%.1f_size_%d-%d-%.1f.png")
  % prefix
  % DSS::imageVersion(imageVersion)
  % (coordinates.rightAscension.sexagesimalHours().hours == 0 ? arSignFix : "")
  % coordinates.rightAscension.sexagesimalHours().hours
  % coordinates.rightAscension.sexagesimalHours().minutes
  % coordinates.rightAscension.sexagesimalHours().seconds
  % (coordinates.declination.sexagesimal().degrees == 0 ? decSignFix : "")
  % coordinates.declination.sexagesimal().degrees
  % coordinates.declination.sexagesimal().minutes
  % coordinates.declination.sexagesimal().seconds
  % size.sexagesimal().degrees
  % size.sexagesimal().minutes
  % size.sexagesimal().seconds;

  return cache_dir / cacheKey;
}


void DSSImage::Private::Image::resize(const fs::path &destination, const DSSImage::ImageOptions &imageOptions)
{
  if(pixels == 0) return;
  auto fullImage = imageSizeMap[DSSImage::Full].file(imageOptions).string();
  if(! fs::exists(fullImage)) return;
  Magick::Image image(fullImage);
  apply_common_options(image);
  image.scale({pixels,pixels});
  image.write(destination.string());
}



string DSSImage::ImageOptions::url() const
{
  double objectRect = size.arcMinutes();
  string arSignFix = coordinates.rightAscension.degrees() < 0 ? "-" : "";
  string decSignFix = coordinates.declination.degrees() < 0 ? "-" : "";

  return format("http://archive.stsci.edu/cgi-bin/dss_search?v=%s&r=%s%d+%d+%.1f&d=%s%d+%d+%.1f&e=J2000&h=%d&w=%df&f=gif&c=none&fov=SM97&v3=")
  % DSS::imageVersion(imageVersion)
  % (coordinates.rightAscension.sexagesimalHours().hours == 0 ? arSignFix : "")
  % coordinates.rightAscension.sexagesimalHours().hours
  % coordinates.rightAscension.sexagesimalHours().minutes
  % coordinates.rightAscension.sexagesimalHours().seconds
  % (coordinates.declination.sexagesimal().degrees == 0 ? decSignFix : "")
  % coordinates.declination.sexagesimal().degrees
  % coordinates.declination.sexagesimal().minutes
  % coordinates.declination.sexagesimal().seconds
  % objectRect % objectRect;
}


void DSSImage::Private::setImageFromCache(shared_ptr<DialogControl::Finish> finishDialogControl)
{
  (void) finishDialogControl;
  setImage(linkFor(file()));
}

void DSSImage::negate()
{
  if(d->negated) {
    d->negated = false;
    d->setImage(d->linkFor(d->file()));
    return;
  }
  d->negated = true;
  d->setImage(d->negate(d->file()));
}

Wt::WLink DSSImage::Private::negate(const boost::filesystem::path &file)
{
  try {
    spLog("notice") << "negating image..";
    Magick::Image image(file.string());
    apply_common_options(image);
    image.negate();
    Magick::Blob blob;
    image.write(&blob, "PNG");
    spLog("notice") << "negating image, wrote " << blob.length() << " bytes";
    vector<uint8_t> data(blob.length());
    const uint8_t *data_c = reinterpret_cast<const uint8_t*>(blob.data());
    copy(data_c, data_c + blob.length(), begin(data));
    return new WMemoryResource("image/jpeg", data, q);
  } catch(exception &e) {
    spLog("error") << "Error negating dss image: " << e.what();
    return linkFor(file);
  }
}



void DSSImage::Private::setImage(const Wt::WLink& link)
{
  _imageLink = link;
  spLog("notice") << "reloading image: " << link.url();
  content->clear();
  content->setPositionScheme(Wt::Relative);
  if(showAnchor) {
    content->addWidget(WW<WAnchor>(link).setTarget(TargetNewWindow).add(WW<WImage>(link).addCss("img-responsive")));
  }
  else {
    content->addWidget(WW<WImage>(link).addCss("img-responsive").onClick([=](const WMouseEvent &e){imageClicked.emit(e); }));
  }
  spLog("notice") << "Loading image file: " << file() ;
  Magick::Image image(file().string());
  if(image.isValid()) {
    imageSize = Size{image.size().width(), image.size().height()};
  } else
  {
    spLog("notice") << "image loading failed";
  }
  _loaded.emit(link);
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

  if(d->negated) {
    return d->negate(d->fullFile());
  }
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
    <div class="container-fluid">
      <table class="col-xs-3">
        <tr><td /><td>${up-button}</td><td /></tr>
        <tr><td>${left-button}</td><td /><td>${right-button}</td></tr>
        <tr><td /><td>${down-button}</td><td /></tr>
      </table>
      <div class="col-xs-7">
        ${tr:imagecontrol-zoom-label}<br />
	  <div class="input-group">${zoom}<span class="input-group-btn">${zoom-button}</span></div>
        <br />
        ${tr:imagecontrol-move-factor-label}<br />
        ${move-factor}<br />
        ${restore-default}<br />
      </div>
    </div>
  )");
  
  dialog->setWidth(500);

  content->addFunction("tr", &WTemplate::Functions::tr);

  WSlider *moveFactor = new WSlider;
  WDoubleSpinBox *zoomLevel = new WDoubleSpinBox();
  zoomLevel->setRange(0, 75);
  zoomLevel->setDecimals(1);
  moveFactor->setRange(0, 100);
  
  auto moveBy = [=](int ar, int dec) {
    if(dialogControl)
      dialogControl->downloading();
    double ratio = static_cast<double>(moveFactor->value()) / 100.;
    double arcMinMove = imageOptions.size.arcMinutes() * ratio;
    imageOptions.coordinates.rightAscension += Angle::arcMinutes(static_cast<double>(ar) * arcMinMove);
    imageOptions.coordinates.declination += Angle::arcMinutes(static_cast<double>(dec) * arcMinMove);
    imageOptions.onViewPortChanged(imageOptions.coordinates, imageOptions.size);
    reload();
  };
  zoomLevel->setValue(imageOptions.size.arcMinutes());
  moveFactor->setValue(10);
  content->bindWidget("zoom-button", WW<WPushButton>(WString::tr("buttons_set")).onClick([=](WMouseEvent){
    if(dialogControl)
      dialogControl->downloading();
    imageOptions.size = Angle::arcMinutes( zoomLevel->value() ); 
    spLog("notice") << "reloading with zoom level: " << imageOptions.size.arcMinutes() << " arcminutes";
    imageOptions.onViewPortChanged(imageOptions.coordinates, imageOptions.size);
    reload();
  }));
  content->bindWidget("move-factor", moveFactor);
  content->bindWidget("zoom", zoomLevel);
  content->bindWidget("up-button", WW<WPushButton>("DEC+").css("btn-sm btn-block btn-move-arrow").onClick([=](WMouseEvent) { moveBy(0, 1 ); }) );
  content->bindWidget("down-button", WW<WPushButton>("DEC-").css("btn-sm btn-block btn-move-arrow").onClick([=](WMouseEvent) { moveBy(0, -1. ) ; }) );
  content->bindWidget("left-button", WW<WPushButton>("AR-").css("btn-sm btn-block btn-move-arrow").onClick([=](WMouseEvent) { moveBy(-1. , 0); }) );
  content->bindWidget("right-button", WW<WPushButton>("AR+").css("btn-sm btn-block btn-move-arrow").onClick([=](WMouseEvent) { moveBy(1 , 0); }) );
  content->bindWidget("restore-default", WW<WPushButton>(WString::tr("restore_default_viewport")).css("btn-sm btn-block").onClick([=](WMouseEvent) {
    dialogControl->downloading();
    imageOptions.coordinates = imageOptions.originalCoordinates.coordinates;
    imageOptions.size = imageOptions.originalCoordinates.size;
    imageOptions.onViewPortChanged(imageOptions.coordinates, imageOptions.size);
    reload();
  }) );
  dialog->contents()->addWidget(content);
  dialogControl.reset(new DialogControl(dialog, content));
  dialog->finished().connect([=](WDialog::DialogCode, _n5) { dialogControl.reset(); });
  dialog->show();
}

Angle DSSImage::fov() const
{
  return d->imageOptions.size;
}


DSSImage::Private::DialogControl::DialogControl(WDialog *dialog, WTemplate *content)
  : dialog(dialog), content(content), modalWidget( WW<WContainerWidget>().css("modal-layer").get() )
{
  wApp->root()->addWidget(modalWidget.get());
}

DSSImage::Private::DialogControl::~DialogControl()
{
  downloadControl(false);
}

void DSSImage::Private::DialogControl::downloadControl(bool downloading)
{
  dialog->setClosable(!downloading);
  if(downloading)
    modalWidget->addStyleClass("modal-layer-dark");
  else
    modalWidget->removeStyleClass("modal-layer-dark");
  for(auto s: vector<string>{"zoom", "zoom-button", "move-factor", "up-button", "down-button", "left-button", "right-button", "restore-default"})
    static_cast<WFormWidget*>(content->resolveWidget(s))->setEnabled(!downloading);
}



void DSSImage::Private::wtDownload()
{
  httpClient.abort();
  httpClient.get(imageOptions.url()); 
}

void DSSImage::Private::download()
{
  content->clear();
  content->addWidget(new WText(WString::tr("dss_downloading_message")));
  content->addWidget(new WBreak);
  content->addWidget(WW<WImage>("http://gulinux.net/loading_animation.gif").addCss("center-block"));

  wtDownload();
}

void DSSImage::Private::curlDownload()
{
    /*progressHandler->progressBar = new WSlider();
    progressHandler->progressBar->setMaximum(100);
    */
      //content->addWidget(progressHandler->progressBar);

    shared_ptr<CurlProgressHandler> progressHandler{new CurlProgressHandler};
    progressHandler->app = wApp;
    fs::path downloadFile = fullFile();

    downloadThread = boost::thread([=] () mutable {
      shared_ptr<DialogControl::Finish> finishDialogControl(new DialogControl::Finish{dialogControl});
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
        curl->get(imageOptions.url());
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
              setImageFromCache(finishDialogControl);
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

DSS::ImageVersion DSSImage::imageVersion() const
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
  container->clear();
  content = new WContainerWidget;
  if(showDSSLink) {
    WAnchor *original = new WAnchor(imageOptions.url(), "Original DSS Image Link");
    original->setInline(false);
    original->setTarget(Wt::TargetNewWindow);
    container->addWidget(original);
  }
  container->addWidget(content);

  if(fs::exists(fullFile() )) {
    shared_ptr<DialogControl::Finish> finishDialogControl(new DialogControl::Finish{dialogControl});
    setImageFromCache(finishDialogControl);
  }
  else {
    download();
  }

}

void DSSImage::addOverlay(const WLink& source)
{
  WImage *imageOverlay = new WImage(source);
  imageOverlay->clicked().connect([=](WMouseEvent){ delete imageOverlay; });
  imageOverlay->addStyleClass("dss-image-overlay img-responsive");
  d->content->addWidget(imageOverlay);
}


WLink DSSImage::dssOriginalLink() const
{
  return d->imageOptions.url();
}

void DSSImage::startDownload()
{
  d->download();
}

Magick::Image& DSSImage::Private::apply_common_options(Magick::Image& image)
{
  image.colorSpace(Magick::GRAYColorspace);
  image.quality(0);
  return image;
}

DSSImage::Size DSSImage::imageSize() const
{
  return d->imageSize;
}
