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

#ifndef DSSPAGE_P_H
#define DSSPAGE_P_H
#include "widgets/dss/dsspage.h"
#include "models/Models"
#include "widgets/dss/dssimage.h"
#include "session.h"
class DSSPage::Private
{
  public:
    Private(const Wt::Dbo::ptr<NgcObject> &object, Session &session, const DSSPage::Options &options, DSSPage *q );
    Wt::Dbo::ptr<NgcObject> object;
    Session &session;
    std::vector<DSS::ImageVersion> imageVersions;
    int nextDSSTypeIndex = 1;
    void setImageType(DSS::ImageVersion, const std::shared_ptr<std::mutex> &downloadMutex);
    Wt::WContainerWidget *imageContainer;
    Wt::WComboBox *typeCombo;
    Wt::WStandardItemModel *typeModel;
    std::function<void()> runOnClose = []{};
    DSSPage::Options options;
    Wt::WContainerWidget *toolbar;
    DSSImage *dssImage = nullptr;
  private:
    class DSSPage *const q;
};
#endif // DSSPAGE_P_H
