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

#ifndef DSSPAGE_H
#define DSSPAGE_H

#include <Wt/WContainerWidget>
#include "c++/dptr.h"
#include "dssimage.h"
#include <functional>
#include <mutex>
class NgcObject;
namespace Wt {
namespace Dbo {
  template<typename T>
  class ptr;
 }
}
class Session;

class DSSPage : public Wt::WContainerWidget
{
  public:
    struct Options {
      std::function<void()> runOnClose = [=]{};
      std::shared_ptr<std::mutex> downloadMutex;
      bool showClose = true;
      bool showTitle = true;
      bool setPath = true;
      bool optionsAsMenu = false;
      DSSImage::ImageSize imageSize = DSSImage::Full;
      static Options embedded(const std::shared_ptr<std::mutex> &downloadMutex = {});
      static Options standalone(std::function<void()> runOnClose = [=]{});
    };

    DSSPage(const Wt::Dbo::ptr<NgcObject> &object, Session &session, const Options &options, Wt::WContainerWidget *parent = 0 );
    ~DSSPage();
    static std::string internalPath(const Wt::Dbo::ptr<NgcObject> &object, Wt::Dbo::Transaction &transaction);
    void toggleInvert();
  private:
    D_PTR;
};

#endif // DSSPAGE_H
