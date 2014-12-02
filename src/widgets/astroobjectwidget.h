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
#ifndef ASTROOBJECTWIDGET_H
#define ASTROOBJECTWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <mutex>
#include "astro_group.h"

class Session;
class Timezone;
class AstroObjectWidget : public Wt::WCompositeWidget {
public:
  explicit AstroObjectWidget(const AstroGroup &astroGroup, Session &session, const std::shared_ptr<std::mutex> &downloadMutex = {}, const std::vector<Wt::WPushButton*> &actionButtons = {}, Wt::WContainerWidget *parent = 0);
  ~AstroObjectWidget();
  void toggleInvert();
  void setDSSVisible(bool visible);
  bool isDSSVisible() const;
  void setCollapsed(bool collapsed);
  bool isCollapsed() const;
  void reload();
private:
  D_PTR;
};

#endif // ASTROOBJECTWIDGET_H


