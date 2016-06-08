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

#ifndef DSSIMAGE_H
#define DSSIMAGE_H

#include <Wt/WCompositeWidget>
#include "c++/dptr.h"
#include <types.h>
#include <mutex>
#include <memory>
#include "dss.h"
#include <boost/filesystem.hpp>

class ViewPort;
class DSSImage : public Wt::WCompositeWidget
{
  public:
    enum ImageSize {
      Full, Mid, Thumb
    };
    struct Size {
      int width, height;
    };
    struct ImageOptions {
      ImageOptions(const Coordinates::Equatorial &coordinates, const Angle &size, DSS::ImageVersion imageVersion, ImageSize imageSize)
      : coordinates(coordinates), size(size), imageVersion(imageVersion), imageSize(imageSize) {}
      Coordinates::Equatorial coordinates;
      Angle size;
      DSS::ImageVersion imageVersion = DSS::phase2_gsc2;
      ImageSize imageSize = Full;
      std::function<void(const Coordinates::Equatorial&, const Angle &size)> onViewPortChanged = [](const Coordinates::Equatorial&, const Angle &size){};
      struct OriginalCoordinates {
        Coordinates::Equatorial coordinates;
        Angle size;
      };
      OriginalCoordinates originalCoordinates;
      std::string url() const;
      boost::filesystem::path file(const boost::filesystem::path &cache_dir, const std::string &prefix = {}) const;
    };
    DSS::ImageVersion imageVersion() const;
    Wt::Signal<> &failed() const;
    Wt::Signal<Wt::WMouseEvent> &imageClicked() const;
    void negate();
    Wt::Signal<Wt::WLink> &imageLoaded() const;
    DSSImage( const ImageOptions &imageOptions, const std::shared_ptr<std::mutex> &downloadMutex = {}, bool anchor = true, bool showDSSLink = true, Wt::WContainerWidget *parent = 0 );
    ~DSSImage();
    void startDownload();
    Wt::WLink fullImageLink() const;
    Wt::WLink dssOriginalLink() const;
    void showImageControls();
    Size imageSize() const;
    void addOverlay(const Wt::WLink &source);
    Angle fov() const;
  private:
    friend class DSSDownloader;
    D_PTR;
};

#endif // DSSIMAGE_H
