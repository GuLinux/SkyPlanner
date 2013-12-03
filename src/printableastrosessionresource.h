/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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
#include "utils/d_ptr.h"

class Session;
class Telescope;
class AstroSession;
class PrintableAstroSessionResource : public Wt::WResource
{
public:
  enum ReportType { HTML, PDF };
  PrintableAstroSessionResource(const Wt::Dbo::ptr<AstroSession> &astroSession, Session &session, Wt::WObject *parent = 0);
  virtual ~PrintableAstroSessionResource();
  virtual void handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response);
  void setRowsSpacing(int spacing);
  void setTelescope(const Wt::Dbo::ptr<Telescope> &telescope);
  void setReportType(ReportType type);
private:
    D_PTR;
};

#endif // PRINTABLEASTROSESSIONRESOURCE_H
