#include "sendfeedbackpage.hpp"
#include "private/sendfeedbackpage_p.hpp"

#include <utils/d_ptr_implementation.h>
#include <Wt-Commons/wt_helpers.h>
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WPushButton>
#include <Wt/Auth/Login>
#include <Wt/Mail/Client>
#include <Wt/Mail/Mailbox>
#include <Wt/Mail/Message>

#include "models/Models"
#include "utils/format.h"
#include "utils/utils.h"
#include "session.h"
#include "skyplanner.h"
#include <boost/algorithm/string.hpp>
#include <Wt/WIOService>
#include <Wt/WJavaScript>

using namespace Wt;
using namespace WtCommons;
using namespace std;
SendFeedbackPage::Private::Private(Session &session, SendFeedbackPage *q): session(session), enableSendButton(q, "enable_send_button"), q(q)
{
}

SendFeedbackPage::SendFeedbackPage(Session &session, Wt::WContainerWidget *parent)
  : WCompositeWidget(parent), d(session, this)
{
  d->content = new WContainerWidget;
  setImplementation(d->content);
  auto loadFeedbackForm = [this,&session] {
    if( !wApp->internalPathMatches("/feedback") || !session.user() ) {
      return;
    }
    auto id = Utils::fromHexString<Dbo::dbo_traits<NgcObject>::IdType>(wApp->internalPathNextPart("/feedback/"));
    Dbo::Transaction t(session);
    d->feedbackForm(session.find<NgcObject>().where("id = ?").bind(id).resultValue());

  };
  wApp->internalPathChanged().connect([this,&session,loadFeedbackForm](const string &, ...) {
    loadFeedbackForm();
  });
  loadFeedbackForm(); 
}

SendFeedbackPage::~SendFeedbackPage()
{
}

string SendFeedbackPage::internalPath(const Wt::Dbo::ptr<NgcObject> &object, Dbo::Transaction *transaction)
{
  if(!object || ! transaction) {
    return "/feedback";
  }

  return format("/feedback/%x/%s") % object.id()
      % Utils::sanitizeForURL(boost::algorithm::join(NgcObject::namesByCatalogueImportance(*transaction, object), "-"));
}

void SendFeedbackPage::Private::feedbackForm(const Wt::Dbo::ptr<NgcObject> &object)
{
  content->clear();
  if(session.login().user().email().empty()) {
    SkyPlanner::instance()->notification(WString::tr("notification_error_title"), WString::tr("feedback_user_without_email_error"), SkyPlanner::Notification::Error);
    wApp->setInternalPath("/", true);
    return;
  }
  content->addWidget(new WText{WString::tr("feedback_label")});
  if(object) {
    Dbo::Transaction t(session);
    content->addWidget(new WText{WString::tr("feedback_label_object_data").arg(boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object), ", "))});
  }
  WTextArea *messageBody = WW<WTextArea>().css("input-block-level");
  WPushButton *sendButton = WW<WPushButton>(WString::tr("buttons_send")).css("btn btn-primary").setEnabled(false);

  sendButton->clicked().connect([=](WMouseEvent) {
    sendButton->disable();
    Mail::Client client;
    Mail::Message message;
    message.setFrom({"skyplanner@gulinux.net", "SkyPlanner"});
    message.setSubject(WString::tr("feedback_email_subject"));


    Dbo::Transaction t(session);
    WString username = session.user()->loginName();
    string userEmail = session.login().user().email();

    message.setReplyTo({userEmail, username});
    string objectData = object ? (format("database id: %d; names: %s") % object.id() % boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object), ", ")).str() : "no object selected";

    WString body = WString::tr("feedback_email_message").arg(username).arg(userEmail).arg(objectData).arg(messageBody->text());
    message.setBody(body);
    
    string adminEmail, adminName;
    wApp->readConfigurationProperty("admin-email", adminEmail);
    wApp->readConfigurationProperty("admin-name", adminName);
    message.addRecipient(Mail::To, {adminEmail, adminName});
    spLog("notice") << "email subject: " << message.subject();
    spLog("notice") << "email body   : " << body;
    if(client.connect()) {
      client.send(message);
      SkyPlanner::instance()->notification(WString::tr("notification_success_title"), WString::tr("feedback_sent_notification"), SkyPlanner::Notification::Success, 10);
    }
    else {
      spLog("error") << "Error connetting to SMTP Agent.";
      SkyPlanner::instance()->notification(WString::tr("notification_error_title"), WString::tr("feedback_sending_error_notification"), SkyPlanner::Notification::Error);
    }
  });
  enableSendButtonConnection.disconnect();
  enableSendButtonConnection = enableSendButton.connect([=](bool enable, ...) {
    sendButton->setEnabled(enable);
  });

  textChanged.setJavaScript(format("function(sender, event) { \
    var enableButton = $('#%s').val().length > 4; \
    if( $('#%s').prop('disabled') == enableButton ) \
      %s;\
  }") % messageBody->id() % sendButton->id() % enableSendButton.createCall("enableButton") );
  messageBody->keyWentUp().connect(textChanged);

  content->addWidget(messageBody);
  content->addWidget(sendButton);
}
