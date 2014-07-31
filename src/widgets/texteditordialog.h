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

#ifndef TEXTEDITORDIALOG_H
#define TEXTEDITORDIALOG_H

#include <Wt/WDialog>
#include "models/ptr_defs.h"
#include "geocoder.h"
#include "Models"

class Session;

class TextEditorDialog : public Wt::WDialog {
public:
  struct SetDescription {
    typedef std::function<void(Wt::Dbo::Transaction &t, const Wt::WString &txt)> EditTextField;
    typedef std::function<std::string(Wt::Dbo::Transaction &t)> GetTextField;
    SetDescription(const std::string &title, const std::string &notification, GetTextField getDescription, EditTextField editTextField, std::function<void()> onUpdate);
    std::string title;
    std::string notification;
    EditTextField editTextField;
    GetTextField getDescription;
    std::function< void() > onUpdate = [] {};
  };
  
  static TextEditorDialog *description(Session& session, const AstroSessionObjectPtr &o, std::function< void() > onUpdate = []{}, Wt::WObject* parent = 0);
  template<typename T>
  static TextEditorDialog *report(Session& session, const T &o, std::function< void() > onUpdate = []{}, std::string title = "report", Wt::WObject* parent = 0) {
    return new TextEditorDialog({title, "notification_report_saved", [=](Wt::Dbo::Transaction &t){ return o->report() ? *o->report() : std::string{}; },
    [=](Wt::Dbo::Transaction &t, const Wt::WString &txt){ o.modify()->setReport(txt.toUTF8());}, onUpdate}, session, parent);
  }

  ~TextEditorDialog();
  TextEditorDialog(const SetDescription &setDescription, Session& session, Wt::WObject* parent = 0);
private:
};
#endif
 
 
