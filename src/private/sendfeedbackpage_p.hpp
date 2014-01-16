#ifndef SENDFEEDBACKPAGE_P_H
#define SENDFEEDBACKPAGE_P_H

#include "sendfeedbackpage.hpp"

class SendFeedbackPage::Private
{
public:
  Private(Session &session, SendFeedbackPage *q);
  Session &session;
  Wt::WContainerWidget *content;

  void feedbackForm(const Wt::Dbo::ptr<NgcObject> &object);
private:
  SendFeedbackPage *q;
};

#endif // SENDFEEDBACKPAGE_H


