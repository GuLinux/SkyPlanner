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


class DSSImage::Private
{
  public:
    Private( const Coordinates::Equatorial &coordinates, const Angle &size, DSSImage::ImageVersion imageVersion, bool autoStartDownload, DSSImage *q );
    Coordinates::Equatorial coordinates;
    Angle size;
    DSSImage::ImageVersion imageVersion;
    boost::filesystem::path cacheFile;
    std::string imageLink() const;
    std::string cacheKey() const;
    void curlDownload();
    void setCacheImage();
    Wt::WContainerWidget *content;
    int retry = 0;
    static std::map<DSSImage::ImageVersion,std::string> imageVersionStrings;
    Wt::Signal<> failed;
    Wt::Signal<Wt::WLink> _loaded;
    bool autoStartDownload;
    Wt::WLink _imageLink;
    bool showAnchor;
    Wt::Signal<Wt::WMouseEvent> imageClicked;
  private:
    class DSSImage *const q;
};
#endif // DSSIMAGE_P_H
