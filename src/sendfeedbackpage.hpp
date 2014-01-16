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


