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
#include <Wt/WDialog>
#include <Wt/WCheckBox>
#include <Wt/WTemplate>
#include "utils/format.h"
#include "utils/utils.h"
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
  spLog("notice") << "Switching to preview version..";
  WContainerWidget *sessionPreviewContainer = WW<WContainerWidget>().css("astroobjects-list");
  setImplementation(sessionPreviewContainer);
  sessionPreviewContainer->setStyleClass("astroobjects-list");
  sessionPreviewContainer->addWidget(WW<WText>(WString("<h3>{1}</h3><h4>{2}</h4>").arg(Utils::htmlEncode(WString::fromUTF8(d->astroGroup.astroSession()->name()))).arg(d->astroGroup.astroSession()->wDateWhen().toString("dddd d MMMM yyyy") )).css("text-center") );
  if(type == PublicReport) {
    Dbo::Transaction t(session);
    auto user = astroGroup.astroSession()->user()->loginName();
    sessionPreviewContainer->addWidget(WW<WText>(WString("<h5 class='text-right'>{1}</h5>").arg(WString::tr("share-report-username").arg(user))) );
  }
  WPushButton *printButton = WW<WPushButton>(WString::tr("buttons_print")).css("btn-info btn-sm");
  printButton->clicked().connect([=](WMouseEvent){
    wApp->doJavaScript("window.print();", false);
    printButton->disable();
    WTimer::singleShot(500, [=](WMouseEvent) {
      printButton->enable();
    });
  });
  WPushButton *backButton = WW<WPushButton>(WString::tr("buttons_back")).css("btn-warning btn-sm").onClick([=](WMouseEvent){ d->backClicked.emit(); });
  WPushButton *invertAllButton = WW<WPushButton>(WString::tr("buttons_invert_all")).css("btn-sm");
  WPushButton *shareButton = WW<WPushButton>(WString::tr("buttons_share")).css("btn-success btn-sm").onClick([=](WMouseEvent){
    WDialog *dialog = new WDialog{WString::tr("buttons_share")};
    dialog->setMinimumSize(700, WLength::Auto);
    WCheckBox *shareCheckBox = WW<WCheckBox>(WString::tr("share-report-enable"));
    dialog->contents()->addWidget(WW<WContainerWidget>().add(shareCheckBox));
    
    auto internalUrl = wApp->bookmarkUrl(format("/report/%x/%s") % astroGroup.astroSession().id() % ::Utils::sanitizeForURL(astroGroup.astroSession()->name()) );
    WContainerWidget *shareText = WW<WContainerWidget>().add(new WText{WString::tr("share-report-message").arg(wApp->makeAbsoluteUrl(internalUrl))});
    
    dialog->contents()->addWidget(shareText);
    
    shareCheckBox->changed().connect([=](_n1){
      Dbo::Transaction t(d->session);
      astroGroup.astroSession().modify()->setReportShared(shareCheckBox->isChecked());
      astroGroup.astroSession().flush();
      
      shareText->setHidden(!astroGroup.astroSession()->reportShared());
    });
    shareText->setHidden(!astroGroup.astroSession()->reportShared());
    shareCheckBox->setChecked(astroGroup.astroSession()->reportShared());
    
    dialog->footer()->addWidget(WW<WPushButton>(WString::tr("Wt.WMessageBox.Ok")).css("btn-primary").onClick([=](WMouseEvent){dialog->accept(); }));
    dialog->show();
  });
  WToolBar *toolbar = WW<WToolBar>().addCss("hidden-print pull-right").addButton(backButton).addButton(invertAllButton).addButton(printButton);
  if(type == Report)
    toolbar->addButton(shareButton);
  sessionPreviewContainer->addWidget(toolbar);

  WContainerWidget *infoWidget = WW<WContainerWidget>().css("astroobjects-info-widget");
  sessionPreviewContainer->addWidget(new PositionDetailsWidget{astroGroup, geoCoderPlace, session, false});

  if(type == Preview) {
    AstroObjectsTable *planetsTable = new AstroObjectsTable(session, {}, false, {}, {AstroObjectsTable::Names, AstroObjectsTable::AR, AstroObjectsTable::DEC, AstroObjectsTable::Constellation, AstroObjectsTable::Magnitude, AstroObjectsTable::AngularSize, AstroObjectsTable::TransitTime, AstroObjectsTable::MaxAltitude});
    planetsTable->addStyleClass("planets-table");
    planetsTable->setResponsive(false);
    WPanel *planetsPanel = new WPanel;
    planetsPanel->setTitle(WString::tr("astrosessiontab_planets_panel"));
    planetsPanel->setCollapsible(true);
    planetsPanel->titleBarWidget()->addStyleClass("hidden-print");
    planetsPanel->setCentralWidget(WW<WContainerWidget>().add(WW<WText>(WString("<h5>{1}</h5>").arg(WString::tr("astrosessiontab_planets_panel"))).css("visible-print") ).add(planetsTable));
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
	report->bindString("report-label", WString::tr("report-label"));
	report->bindString("report", Utils::htmlEncode(WString::fromUTF8(*d->astroGroup.astroSession()->report()), Utils::EncodeNewLines));
	reportContainer->addWidget(report);
      }
    };
    displayReport();
    if(type != PublicReport) {
      toolbar->addButton(WW<WPushButton>(WString::tr("astrosessiontab_set_report")).css("btn-primary btn-sm hidden-pront").onClick([=,&session](WMouseEvent){
	TextEditorDialog::report(session, astroGroup.astroSession(), displayReport, "astrosessiontab_set_report")->show();
      }));
    }
  }
  sessionPreviewContainer->addWidget(WW<WText>(WString::tr("dss-embed-menu-info-message")).css("hidden-print"));

  shared_ptr<mutex> downloadImagesMutex(new mutex);
  Dbo::Transaction t(session);

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
  shared_ptr<set<AstroObjectWidget*>> astroObjectWidgets(new set<AstroObjectWidget*>());
  AstroObjectWidget *astroObjectWidget = nullptr;
  for(auto objectelement: sessionObjects) {
    WPushButton *hideButton = WW<WPushButton>(WString::tr("buttons_hide")).css("btn-xs btn-warning");
    WPushButton *collapseButton = WW<WPushButton>(WString::tr("buttons_collapse")).css("btn-xs");
    WPushButton *hideDSSButton = WW<WPushButton>(WString::tr("buttons_hide_dss")).css("btn-xs");
    WPushButton *editDescriptionButton = WW<WPushButton>(WString::tr("astroobject_actions_edit_description")).css("btn-xs");
    vector<WPushButton*> actionButtons = { collapseButton, hideDSSButton, hideButton };
    if(type != PublicReport)
      actionButtons.insert(actionButtons.begin(), editDescriptionButton);
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
    hideButton->clicked().connect([=](WMouseEvent){astroObjectWidgets->erase(astroObjectWidget); delete astroObjectWidget; });
    hideDSSButton->clicked().connect([=](WMouseEvent){ astroObjectWidget->setDSSVisible(!astroObjectWidget->isDSSVisible()); hideDSSButton->setText(WString::tr( astroObjectWidget->isDSSVisible() ? "buttons_hide_dss" : "buttons_show_dss" ));  });
    collapseButton->clicked().connect([=](WMouseEvent) { astroObjectWidget->setCollapsed(!astroObjectWidget->isCollapsed()); });
    editDescriptionButton->clicked().connect([=,&session](WMouseEvent) {
      Dbo::Transaction t(d->session);
      TextEditorDialog::description(session, objectelement.first, [=] { astroObjectWidget->reload(); } )->show();
    });
    astroObjectWidgets->insert(astroObjectWidget);
    sessionPreviewContainer->addWidget(astroObjectWidget);
  }
  if(astroObjectWidget)
    astroObjectWidget->addStyleClass("astroobject-last-list-item");
  invertAllButton->clicked().connect([=](WMouseEvent){ for(auto a: *astroObjectWidgets) a->toggleInvert(); } );
}

Signal<> &AstroSessionPreview::backClicked() const
{
  return d->backClicked;
}

AstroSessionPreview::~AstroSessionPreview()
{
}
