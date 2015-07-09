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
#include "Models"
#include "private/astrosessionpreview_p.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/d_ptr_implementation.h"
#include "skyplanner.h"
#include <Wt/Utils>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WTimer>
#include <Wt/WToolBar>
#include <Wt/WStackedWidget>
#include <Wt/WTemplate>
#include <Wt/WLabel>
#include "widgets/astroobjectstable.h"
#include <mutex>
#include "widgets/astroobjectwidget.h"
#include "widgets/positiondetailswidget.h"
#include "widgets/texteditordialog.h"
#include "widgets/instrumentstable.h"
#include <Wt/WDialog>
#include <Wt/WCheckBox>
#include <Wt/WTemplate>
#include "utils/format.h"
#include "utils/utils.h"
#include <Wt/WLocalDateTime>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include "Wt-Commons/wt_utils.h"
using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroSessionPreview::Private::Private(const AstroGroup& astroGroup, Session& session, AstroSessionPreview* q)
  : astroGroup(astroGroup), session(session), q(q)
{
}

AstroSessionPreview::AstroSessionPreview(const AstroGroup& astroGroup, const GeoCoder::Place &geoCoderPlace, Session& session, list<ObjectAction> actions, Type type, Wt::WContainerWidget* parent)
  : WCompositeWidget(parent), d(astroGroup, session, this)
{
  Dbo::Transaction t(session);
  spLog("notice") << "Switching to preview version..";
  
  string typeString = type == Report ? "report" : "sessionpreview";

  WContainerWidget *sessionPreviewContainer = WW<WContainerWidget>().css("astroobjects-list");
  setImplementation(sessionPreviewContainer);
  sessionPreviewContainer->setStyleClass("astroobjects-list");
  sessionPreviewContainer->addWidget(WW<WText>("<h3>{1}</h3><h4>{2}</h4>"_ws
    | Utils::htmlEncode(WString::fromUTF8(d->astroGroup.astroSession()->name())) 
    | WLocalDateTime(astroGroup.astroSession()->wDateWhen().date(), astroGroup.astroSession()->wDateWhen().time()).toString("dddd dd MMMM yyyy")
  ).css("text-center") );
  if(type == PublicReport || type == PublicPreview) {
    Dbo::Transaction t(session);
    auto user = astroGroup.astroSession()->user()->loginName();
    sessionPreviewContainer->addWidget(WW<WText>("<h5 class='text-right'>{1}</h5>"_ws | WString::tr("share-%s-username"_s % typeString ) | user));
  }
  WPushButton *printButton = WW<WPushButton>("buttons_print"_wtr).css("btn-info btn-sm");
  printButton->clicked().connect([=](WMouseEvent){
    wApp->doJavaScript("window.print();", false);
    printButton->disable();
    WTimer::singleShot(500, [=](WMouseEvent) {
      printButton->enable();
    });
  });
  WPushButton *backButton = WW<WPushButton>("buttons_back"_wtr).css("btn-warning btn-sm").onClick([=](WMouseEvent){ d->backClicked.emit(); });
  WPushButton *invertAllButton = WW<WPushButton>("buttons_invert_all"_wtr).css("btn-sm");
  WPushButton *shareButton = WW<WPushButton>("buttons_share"_wtr).css("btn-success btn-sm").onClick([=](WMouseEvent){
    WDialog *dialog = new WDialog{"buttons_share"_wtr};
    dialog->setMinimumSize(700, WLength::Auto);
    WCheckBox *shareCheckBox = WW<WCheckBox>(WString::tr("share-%s-enable"_s % typeString));
    dialog->contents()->addWidget(WW<WContainerWidget>().add(shareCheckBox));
    auto internalUrl = wApp->bookmarkUrl("/%s/%x/%s"_s % typeString % astroGroup.astroSession().id() % ::Utils::sanitizeForURL(astroGroup.astroSession()->name()) );
    
    
    WTemplate *shareToolbox = WW<WTemplate>();
    shareToolbox->setTemplateText(R"(
<!-- I got these buttons from simplesharebuttons.com -->
<div id="share-buttons">
<!-- Facebook -->
<a href="http://www.facebook.com/sharer.php?u=${page-url}" target="_blank"><img src="http://www.simplesharebuttons.com/images/somacro/facebook.png" alt="Facebook" /></a>
 
<!-- Twitter -->
<a href="http://twitter.com/share?url=${page-url}&text=${page-title}&hashtags=skyplanner" target="_blank"><img src="http://www.simplesharebuttons.com/images/somacro/twitter.png" alt="Twitter" /></a>
 
<!-- Google+ -->
<a href="https://plus.google.com/share?url=${page-url}" target="_blank"><img src="http://www.simplesharebuttons.com/images/somacro/google.png" alt="Google" /></a>
 
<!-- Pinterest -->
<a href="javascript:void((function()%7Bvar%20e=document.createElement('script');e.setAttribute('type','text/javascript');e.setAttribute('charset','UTF-8');e.setAttribute('src','http://assets.pinterest.com/js/pinmarklet.js?r='+Math.random()*99999999);document.body.appendChild(e)%7D)());"><img src="http://www.simplesharebuttons.com/images/somacro/pinterest.png" alt="Pinterest" /></a>
</div>
    )", XHTMLUnsafeText);
    shareToolbox->bindString("page-url", wApp->makeAbsoluteUrl(internalUrl));
    shareToolbox->bindString("page-title", astroGroup.astroSession()->name() );
    
    WContainerWidget *shareText = WW<WContainerWidget>().add(new WText{WString::tr("share-%s-message"_s % typeString) | wApp->makeAbsoluteUrl(internalUrl)}).add(shareToolbox);

    dialog->contents()->addWidget(shareText);
    
    auto isShared = [=] {
      if(type == Report)
	return astroGroup.astroSession()->reportShared();
      if(type == Preview)
	return astroGroup.astroSession()->previewShared();
      return false;
    };

    shareCheckBox->changed().connect([=](_n1){
      Dbo::Transaction t(d->session);
      if(type == Report)
	astroGroup.astroSession().modify()->setReportShared(shareCheckBox->isChecked());
      if(type == Preview)
	astroGroup.astroSession().modify()->setPreviewShared(shareCheckBox->isChecked());
	
      astroGroup.astroSession().flush();
      
      shareText->setHidden(!isShared());
    });
    shareText->setHidden(!isShared());
    shareCheckBox->setChecked(isShared());
    
    dialog->footer()->addWidget(WW<WPushButton>("Wt.WMessageBox.Ok"_wtr).css("btn-primary").onClick([=](WMouseEvent){dialog->accept(); }));
    dialog->show();
  });
  WPushButton *copyButton = WW<WPushButton>("buttons_copy"_wtr).css("btn-sm").onClick([=,&session](WMouseEvent){
    Dbo::Transaction t(session);
    auto dialog = new WMessageBox("buttons_copy"_wtr, "copy_session_name"_wtr, Question, Ok | Cancel, this);
    auto sessionName = new WLineEdit( "session_copy_of"_wtr | astroGroup.astroSession()->name());
    dialog->contents()->addWidget(sessionName);
    dialog->button(Ok)->clicked().connect([=](WMouseEvent){ dialog->accept();});
    dialog->button(Ok)->setEnabled(!sessionName->text().empty());
    sessionName->keyWentUp().connect([=](WKeyEvent){ dialog->button(Ok)->setEnabled(! sessionName->text().empty() ); });
    dialog->button(Cancel)->clicked().connect([=](WMouseEvent){ dialog->reject();});
    dialog->finished().connect([=,&session](int r, _n5){
      Scope scope([=]{ delete dialog; } );
      if(r != WDialog::Accepted || sessionName->text().empty())
	return;
      Dbo::Transaction t(session);
      auto newSession = new AstroSession(sessionName->text().toUTF8(), astroGroup.astroSession()->when(), session.user());
      newSession->setPosition(astroGroup.astroSession()->position());
      session.add(newSession);
      auto query = session.find<AstroSessionObject>().where("astro_session_id = ?").bind(astroGroup.astroSession().id());
      if(type == Report)
	query.where("observed = ?").bind(true);
      for(auto o: query.resultList()) {
	newSession->astroSessionObjects().insert(new AstroSessionObject{o->ngcObject()});
      }
      WMessageBox::show("session_copied"_wtr, "session_copied_text"_wtr | newSession->name(), Ok);
      d->sessionsChanged.emit();
    });
    dialog->show();
  });
  WToolBar *toolbar = WW<WToolBar>().addCss("hidden-print pull-right").addButton(backButton).addButton(invertAllButton).addButton(printButton);
  if(type == Report || type == Preview)
    toolbar->addButton(shareButton);
  sessionPreviewContainer->addWidget(toolbar);

  sessionPreviewContainer->addWidget(new PositionDetailsWidget{astroGroup, geoCoderPlace, session});
  if(type == Preview && session.user()->instruments_ok() ) {
    InstrumentsTable *instrumentsTable = new InstrumentsTable(session.user(), session);
    WPanel *instrumentsPanel = new WPanel;
    instrumentsPanel->setTitle("astrosessiontab_instruments_panel"_wtr);
    instrumentsPanel->setCollapsible(true);
    instrumentsPanel->setCollapsed(true);
    instrumentsPanel->titleBarWidget()->addStyleClass("hidden-print");
    instrumentsPanel->setCentralWidget(WW<WContainerWidget>().add(WW<WText>("<h5>{1}</h5>"_ws | "astrosessiontab_instruments_panel"_wtr).css("visible-print") ).add(instrumentsTable));
    sessionPreviewContainer->addWidget(instrumentsPanel);
  }

  if(type == Preview || type == PublicPreview) {
    AstroObjectsTable *planetsTable = new AstroObjectsTable(session, {}, AstroObjectsTable::NoFiltersButton, {}, AstroObjectsTable::PlanetColumns());
    planetsTable->addStyleClass("planets-table");
    planetsTable->setResponsive(false);
    WPanel *planetsPanel = new WPanel;
    planetsPanel->setTitle("astrosessiontab_planets_panel"_wtr);
    planetsPanel->setCollapsible(true);
    planetsPanel->setCollapsed(true);
    planetsPanel->titleBarWidget()->addStyleClass("hidden-print");
    planetsPanel->setCentralWidget(WW<WContainerWidget>().add(WW<WText>("<h5>{1}</h5>"_ws | "astrosessiontab_planets_panel"_wtr).css("visible-print") ).add(planetsTable));
    sessionPreviewContainer->addWidget(planetsPanel);
    planetsTable->planets(astroGroup.astroSession(), astroGroup.timezone);
  } else {
    WContainerWidget *reportContainer = WW<WContainerWidget>();
    sessionPreviewContainer->addWidget(reportContainer);
    auto displayReport = [=,&session] {
      reportContainer->clear();
      if(d->astroGroup.astroSession()->report()) {
	WTemplate *report = WW<WTemplate>(R"(
	  <dl class="dl-horizontal">
	    <dt>${report-label}</dt>
	    <dd>${report}</dd>
	  </dl>
	)").css("well");
	report->bindString("report-label", "report-label"_wtr);
	report->bindString("report", Utils::htmlEncode(WString::fromUTF8(*d->astroGroup.astroSession()->report()), Utils::EncodeNewLines));
	reportContainer->addWidget(report);
      }
    };
    displayReport();
    if(type == Report || type == Preview) {
      toolbar->addButton(WW<WPushButton>("astrosessiontab_set_report"_wtr).css("btn-primary btn-sm hidden-pront").onClick([=,&session](WMouseEvent){
	TextEditorDialog::report(session, astroGroup.astroSession(), displayReport, "astrosessiontab_set_report")->show();
      }));
    }
  }
  if(session.user()) {
    toolbar->addButton(copyButton);
  }
  sessionPreviewContainer->addWidget(WW<WText>("dss-embed-menu-info-message"_wtr).css("hidden-print"));

  shared_ptr<mutex> downloadImagesMutex(new mutex);

  typedef pair<AstroSessionObjectPtr, Ephemeris::BestAltitude> AstroSessionObjectElement;
  vector<AstroSessionObjectElement> sessionObjects;
  // TODO: filter this section too?
  {
    Ephemeris ephemeris({d->astroGroup.astroSession()->position().latitude, d->astroGroup.astroSession()->position().longitude}, d->astroGroup.timezone);
    AstroSessionObject::generateEphemeris(ephemeris, d->astroGroup.astroSession(), d->astroGroup.timezone, t);
    auto query = session.query<AstroSessionObjectPtr>("select a from astro_session_object a inner join objects on a.objects_id = objects.id")
      .where("astro_session_id = ?").bind(d->astroGroup.astroSession().id());
    if(type == Report || type == PublicReport)
      query.where("observed = ?").bind(true);
    query.orderBy("transit_time ASC, ra asc, dec asc, constellation_abbrev asc");


    auto sessionObjectsDbCollection = query.resultList();
    transform(begin(sessionObjectsDbCollection), end(sessionObjectsDbCollection), back_inserter(sessionObjects), [=,&ephemeris,&t](const AstroSessionObjectPtr &o){
      return AstroSessionObjectElement{o, o->bestAltitude(ephemeris, d->astroGroup.timezone)};
    });
  }
  AstroObjectWidget *astroObjectWidget = nullptr;
  for(auto objectelement: sessionObjects) {
    WPushButton *hideButton = WW<WPushButton>("buttons_hide"_wtr).css("btn-xs btn-warning");
    WPushButton *collapseButton = WW<WPushButton>("buttons_collapse"_wtr).css("btn-xs");
    WPushButton *hideDSSButton = WW<WPushButton>("buttons_hide_dss"_wtr).css("btn-xs");
    WPushButton *editReportButton = WW<WPushButton>("Report"_wtr).css("btn-xs");
    WPushButton *editDescriptionButton = WW<WPushButton>("astroobject_actions_edit_description"_wtr).css("btn-xs");
    vector<WPushButton*> actionButtons = { collapseButton, hideDSSButton, hideButton };
    if(type == Report || type == Preview)
      actionButtons.insert(actionButtons.begin(), editDescriptionButton);
    if(type == Report)
        actionButtons.insert(actionButtons.begin(), editReportButton);
    vector<pair<ObjectAction, WPushButton*>> objectActionButtons;
    for(auto action: actions) {
      WPushButton *button = WW<WPushButton>(WString::tr(action.buttonName)).addCss("btn-xs").addCss(action.buttonStyle);
      actionButtons.push_back(button);
      objectActionButtons.push_back({action, button});
    }
    astroObjectWidget = new AstroObjectWidget({objectelement.first, d->astroGroup.telescope, d->astroGroup.timezone}, session, downloadImagesMutex, actionButtons);
    astroObjectWidget->addStyleClass("astroobject-list-item");
    for(auto o: objectActionButtons) {
      o.second->clicked().connect([=](WMouseEvent){ o.first.clicked(objectelement.first, astroObjectWidget); });
    }
    hideButton->clicked().connect([=](WMouseEvent){ delete astroObjectWidget; });
    hideDSSButton->clicked().connect([=](WMouseEvent){ astroObjectWidget->setDSSVisible(!astroObjectWidget->isDSSVisible()); hideDSSButton->setText(WString::tr( astroObjectWidget->isDSSVisible() ? "buttons_hide_dss" : "buttons_show_dss" ));  });
    collapseButton->clicked().connect([=](WMouseEvent) { astroObjectWidget->setCollapsed(!astroObjectWidget->isCollapsed()); });
    editDescriptionButton->clicked().connect([=,&session](WMouseEvent) {
      Dbo::Transaction t(d->session);
      TextEditorDialog::description(session, objectelement.first, [=] { astroObjectWidget->reload(); } )->show();
    });
    editReportButton->clicked().connect([=](WMouseEvent) {
     Dbo::Transaction t(d->session);
     TextEditorDialog::report(d->session, objectelement.first, [=] { astroObjectWidget->reload(); })->show();
    });
    sessionPreviewContainer->addWidget(astroObjectWidget);
    invertAllButton->clicked().connect(astroObjectWidget, &AstroObjectWidget::toggleInvert);
  }
  if(astroObjectWidget)
    astroObjectWidget->addStyleClass("astroobject-last-list-item");
}

Signal<> &AstroSessionPreview::backClicked() const
{
  return d->backClicked;
}

Signal<> &AstroSessionPreview::sessionsChanged() const
{
  return d->sessionsChanged;
}



AstroSessionPreview::~AstroSessionPreview()
{
}
