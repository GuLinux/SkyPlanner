#ifndef SENDFEEDBACKPAGE_P_H
#define SENDFEEDBACKPAGE_P_H

#include "sendfeedbackpage.hpp"
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


