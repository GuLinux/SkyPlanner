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
#include "texteditordialog.h"
#include "Wt-Commons/wt_helpers.h"
#include "skyplanner.h"
#include <Wt/WText>
#include <Wt/WLink>
#include <Wt/WAnchor>
#include <Wt/WPushButton>
#include <Wt/WTextArea>
#include "utils/format.h"
#include "session.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

TextEditorDialog::SetDescription::SetDescription(const string &title, const string &notification, GetTextField getDescription, EditTextField editTextField, function<void()> onUpdate)
    : title(title), notification(notification), getDescription(getDescription), editTextField(editTextField), onUpdate(onUpdate) 
{
}

TextEditorDialog *TextEditorDialog::description(Session &session, const AstroSessionObjectPtr &o, std::function< void() > onUpdate, WObject *parent)
{
  return new TextEditorDialog({"object_notes", "notification_description_saved", [=](Dbo::Transaction &t){ return o->description(); },
    [=](Dbo::Transaction &t, const WString &txt){ o.modify()->setDescription(txt.toUTF8());}, onUpdate}, session, parent);
}

TextEditorDialog::TextEditorDialog(const SetDescription &setDescription, Session& session, WObject* parent)
{
  Dbo::Transaction t(session);
  setWindowTitle(WString::tr(setDescription.title));
  resize({60, WLength::Percentage}, {50, WLength::Percentage});
  WTextArea *descriptionTextArea = WW<WTextArea>(WString::fromUTF8(setDescription.getDescription(t))).css("input-block-level resize-none");
  descriptionTextArea->setHeight({100, WLength::Percentage});
  contents()->addWidget(descriptionTextArea);
  footer()->addWidget(WW<WPushButton>(WString::tr("buttons_close")).css("btn-sm").onClick([=](WMouseEvent){ reject(); }));
  footer()->addWidget( WW<WPushButton>(WString::tr("buttons_save")).css("btn-sm btn-primary").onClick([=,&session](WMouseEvent){
    accept();
    Dbo::Transaction t(session);
    setDescription.editTextField(t, descriptionTextArea->text());
    SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr(setDescription.notification), SkyPlanner::Notification::Success, 5);
    setDescription.onUpdate();
  }));
}

TextEditorDialog::~TextEditorDialog()
{
}
