/*
 * <one line to give the program's name and a brief idea of what it does.>
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

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include "c++/dptr.h"
#include <Wt/WString>
#include <Wt/WWidget>
#include <Wt/WContainerWidget>
class Notification
{
public:
      enum Type { Alert, Error, Success, Information };
      Notification(const Wt::WString &title, Wt::WWidget *content, Type type, bool addCloseButton, const std::string &categoryTag = {}, Wt::WContainerWidget *parent = 0);
      ~Notification();
      void close();
      bool valid() const;
      Wt::Signal<> &closed() const;
      Wt::WWidget *widget() const;
      std::string categoryTag() const;
private:
    D_PTR
};

#endif // NOTIFICATION_H
