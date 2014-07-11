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

#ifndef DSSIMAGE_P_H
#define DSSIMAGE_P_H
#include "widgets/dssimage.h"
#include <Wt/WFileResource>
#include <boost/filesystem/path.hpp>
#include <Wt/WLink>
#include <boost/thread.hpp>
#include <Wt/Http/Client>

class DSSImage::Private
{
  public:
    struct Image {
      uint32_t pixels;
      std::string prefix;
      boost::filesystem::path file(const DSSImage::ImageOptions &imageOptions);
      private:
      void resize(const boost::filesystem::path &destination, const DSSImage::ImageOptions &imageOptions);
    };
    
    class DialogControl {
    public:
      DialogControl(Wt::WDialog *dialog, Wt::WTemplate *content);
      void downloading() { downloadControl(true); }
      void downloadFinished() { downloadControl(false); }
    private:
      void downloadControl(bool downloading);
      Wt::WTemplate *content;
      Wt::WDialog *dialog;
      std::shared_ptr<Wt::WContainerWidget> modalWidget;
    };
    static std::map<DSSImage::ImageSize, Image> imageSizeMap;
    static std::map<DSS::ImageVersion,std::string> imageVersionStrings;

    Private(const DSSImage::ImageOptions &imageOptions, const std::shared_ptr<std::mutex> &downloadMutex, DSSImage *q );
    DSSImage::ImageOptions imageOptions;

    std::string imageLink() const;
    void download();
    void curlDownload();
    void wtDownload();
    void save(const boost::system::error_code &errorCode, const Wt::Http::Message &httpMessage);
    void setImageFromCache();
    Wt::WContainerWidget *content;
    int retry = 0;


    boost::filesystem::path file(DSSImage::ImageSize imageSize) const;
    boost::filesystem::path file() const { return file(imageOptions.imageSize); }
    boost::filesystem::path fullFile() const { return file(Full); }

    Wt::WLink linkFor(const boost::filesystem::path &file) const;

    void showImageController();
    void reload();
    Wt::WContainerWidget *container;
    bool showDSSLink;
    

    Wt::Signal<> failed;
    Wt::Signal<Wt::WLink> _loaded;
    std::shared_ptr<std::mutex> downloadMutex;
    Wt::WLink _imageLink;
    bool showAnchor;
    Wt::Signal<Wt::WMouseEvent> imageClicked;
    boost::thread downloadThread;
    bool aborted = false;
    Wt::Http::Client httpClient;
    std::shared_ptr<DialogControl> dialogControl;
  private:
    class DSSImage *const q;
};
#endif // DSSIMAGE_P_H
