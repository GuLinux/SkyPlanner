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
#ifndef SENDFEEDBACKPAGE_H
#define SENDFEEDBACKPAGE_H

#include "Models"
#include <Wt/WCompositeWidget>

class NgcObject;
class Session;
class SendFeedbackPage : public Wt::WCompositeWidget {
public:
  explicit SendFeedbackPage(Session &session, Wt::WContainerWidget *parent = 0);
  ~SendFeedbackPage();
  static std::string internalPath(const Wt::Dbo::ptr<NgcObject> &object = Wt::Dbo::ptr<NgcObject>(), Wt::Dbo::Transaction *transaction = nullptr);
private:
  D_PTR;
};

#endif // SENDFEEDBACKPAGE_H


