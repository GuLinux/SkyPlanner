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

#ifndef DSSIMAGE_H
#define DSSIMAGE_H

#include <Wt/WCompositeWidget>
#include "utils/d_ptr.h"
#include <types.h>
#include <mutex>
#include <memory>

class DSSImage : public Wt::WCompositeWidget
{
  public:
    enum ImageVersion {
      poss2ukstu_red,
      poss2ukstu_blue,
      poss2ukstu_ir,
      poss1_red,
      poss1_blue,
      quickv,
      phase2_gsc2,
      phase2_gsc1,
    };
    enum ImageSize {
      Full, Mid, Thumb
    };
    struct ImageOptions {
      ImageOptions(const Coordinates::Equatorial &coordinates, const Angle &size, ImageVersion imageVersion, ImageSize imageSize)
      : coordinates(coordinates), size(size), imageVersion(imageVersion), imageSize(imageSize) {}
      Coordinates::Equatorial coordinates;
      Angle size;
      ImageVersion imageVersion = phase2_gsc2;
      ImageSize imageSize = Full;
      std::function<void(const Coordinates::Equatorial&, const Angle &size)> onViewPortChanged = [](const Coordinates::Equatorial&, const Angle &size){};
    };
    static ImageVersion imageVersion(const std::string &version);
    static std::string imageVersion(const ImageVersion &version);
    static std::vector<ImageVersion> versions();
    Wt::Signal<> &failed() const;
    Wt::Signal<Wt::WMouseEvent> &imageClicked() const;
    Wt::Signal<Wt::WLink> &imageLoaded() const;
    DSSImage( const ImageOptions &imageOptions, const std::shared_ptr<std::mutex> &downloadMutex = {}, bool anchor = true, bool showDSSLink = true, Wt::WContainerWidget *parent = 0 );
    ~DSSImage();
    void startDownload();
    Wt::WLink fullImageLink() const;
    Wt::WLink dssOriginalLink() const;
    void showImageControls();
  private:
    D_PTR;
};

#endif // DSSIMAGE_H
