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
#ifndef PRINTABLEASTROSESSIONRESOURCE_H
#define PRINTABLEASTROSESSIONRESOURCE_H

#include <Wt/WResource>
#include "c++/dptr.h"
#include "types.h"
#include "geocoder.h"
#include "ptr_defs.h"

class Session;
class ExportAstroSessionResource : public Wt::WResource
{
public:
  enum ReportType { HTML, PDF, CSV, KStars, CartesDuCiel };
  ExportAstroSessionResource(const AstroSessionPtr &astroSession, Session &session, Timezone timezone, Wt::WObject *parent = 0);
  virtual ~ExportAstroSessionResource();
  virtual void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response);
  void setRowsSpacing(int spacing);
  void setTelescope(const TelescopePtr &telescope);
  void setReportType(ReportType type);
  void setFontScale(double fontScale);
  void setTimezone(const Timezone &timezone);
  void setNamesLimit(int namesLimit);
  void setPlace(const GeoCoder::Place &place);
private:
    D_PTR;
};

#endif // PRINTABLEASTROSESSIONRESOURCE_H

