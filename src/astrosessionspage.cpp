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

#include "astrosessionspage.h"
#include "private/astrosessionspage_p.h"
#include "utils/d_ptr_implementation.h"
#include "Wt-Commons/wt_helpers.h"
#include "session.h"
#include "astrosessionslisttab.h"
#include <Wt/WTabWidget>

using namespace Wt;
using namespace WtCommons;
using namespace std;
AstroSessionsPage::Private::Private(Session& session, AstroSessionsPage* q) : session(session), q(q)
{
}

AstroSessionsPage::~AstroSessionsPage()
{
}

AstroSessionsPage::AstroSessionsPage(Session &session, WContainerWidget* parent)
    : d(session, this)
{
  WTabWidget *tabs = new WTabWidget(this);
  tabs->addTab(new AstroSessionsListTab(session), "Sessions List");
}
