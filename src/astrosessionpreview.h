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
#ifndef ASTROSESSIONPREVIEW_H
#define ASTROSESSIONPREVIEW_H

#include <Wt/WCompositeWidget>
#include "utils/d_ptr.h"
#include "models/ptr_defs.h"
#include "geocoder.h"

class Session;
class AstroObjectWidget;
class AstroSessionPreview : public Wt::WCompositeWidget {
public:
  enum Type{ Preview, Report, PublicReport, PublicPreview };
  struct ObjectAction {
    std::string buttonName;
    std::string buttonStyle;
    std::function<void(const AstroSessionObjectPtr &,AstroObjectWidget *)> clicked;
  };
  ~AstroSessionPreview();
  AstroSessionPreview(const AstroGroup& astroGroup, const GeoCoder::Place &geoCoderPlace, Session& session, std::list<ObjectAction> actions = {}, Type type = Preview, Wt::WContainerWidget* parent = 0);
  Wt::Signal<> &backClicked() const;
private:
  D_PTR;
};
#endif
