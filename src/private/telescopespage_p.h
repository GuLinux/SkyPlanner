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

#ifndef TELESCOPESPAGE_P_H
#define TELESCOPESPAGE_P_H
#include "telescopespage.h"

namespace Wt {
  class WCheckBox;
}

class TelescopesPage::Private
{
  public:
    Private( Session &session, TelescopesPage *q );
    Session &session;
    void loginChanged();
    void populateTelescopes();
    void populateEyepieces();
    void populateFocalMultipliers();
    void setupTelescopesTable();
    void setupEyepiecesTable();
    void setupFocalMultipliersTable();
    Wt::WTable *telescopesTable;
    Wt::WTable *eyepiecesTable;
    Wt::WTable *focalMultipliersTable;
    Wt::WCheckBox *isDefault;
    Wt::Signal<> changed;
  private:
    class TelescopesPage *const q;
};

#endif // TELESCOPESPAGE_P_H
