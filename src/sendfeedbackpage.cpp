#include "sendfeedbackpage.hpp"
#include "private/sendfeedbackpage_p.hpp"

#include <utils/d_ptr_implementation.h>
#include <Wt-Commons/wt_helpers.h>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/Auth/Login>

#include "models/Models"
#include "utils/format.h"
#include "utils/utils.h"
#include "session.h"
#include "astroplanner.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;
SendFeedbackPage::Private::Private(Session &session, SendFeedbackPage *q): session(session), q(q)
{
}

SendFeedbackPage::SendFeedbackPage(Session &session, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), d(session, this)
{
  d->content = new WContainerWidget;
  setImplementation(d->content);
  wApp->internalPathChanged().connect([this,&session](const string &newPath, ...) {
    if( !wApp->internalPathMatches("/feedback") || !session.user() ) {
      return;
    }
    auto id = Utils::fromHexString<Dbo::dbo_traits<NgcObject>::IdType>(wApp->internalPathNextPart("/feedback/"));
    Dbo::Transaction t(session);
    d->feedbackForm(session.find<NgcObject>().where("id = ?").bind(id).resultValue());
  });
}

SendFeedbackPage::~SendFeedbackPage()
{
}

string SendFeedbackPage::internalPath(const Wt::Dbo::ptr<NgcObject> &object)
{
  if(!object) {
    return "/feedback";
  }
  return format("/feedback/%x") % object.id();
}

void SendFeedbackPage::Private::feedbackForm(const Wt::Dbo::ptr<NgcObject> &object)
{
  content->clear();
  if(session.login().user().email().empty()) {
    AstroPlanner::instance()->notification(WString::tr("notification_error_title"), WString::tr("feedback_user_without_email_error"), AstroPlanner::Error);
    wApp->setInternalPath("/", true);
    return;
  }
  content->addWidget(new WText{"Hello"});
}
