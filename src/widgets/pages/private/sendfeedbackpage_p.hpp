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
#ifndef SENDFEEDBACKPAGE_P_H
#define SENDFEEDBACKPAGE_P_H

#include "widgets/pages/sendfeedbackpage.hpp"
#include <Wt/WJavaScript>
class SendFeedbackPage::Private
{
public:
  Private(Session &session, SendFeedbackPage *q);
  Session &session;
  Wt::WContainerWidget *content;

  Wt::JSlot textChanged;
  Wt::JSignal<bool> enableSendButton;
  Wt::Signals::connection enableSendButtonConnection;

  void feedbackForm(const Wt::Dbo::ptr<NgcObject> &object);
private:
  SendFeedbackPage *q;
};

#endif // SENDFEEDBACKPAGE_H


