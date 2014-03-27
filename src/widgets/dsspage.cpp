/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#include "dsspage.h"
#include "private/dsspage_p.h"
#include "utils/d_ptr_implementation.h"
#include "utils/format.h"
#include "utils/utils.h"
#include <Wt/WApplication>
#include <Wt-Commons/wt_helpers.h>
#include "models/Models"
#include <Wt/WComboBox>
#include <Wt/WText>
#include <Wt/WTimer>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WAnchor>
#include <Wt/Utils>
#include <boost/algorithm/string/join.hpp>
#include <Wt/WApplication>
#include <Wt/WEnvironment>
#include <Wt/WTemplate>
#include <Wt/WPopupMenu>
#include "skyplanner.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;



DSSPage::Private::Private(const NgcObjectPtr &object, Session &session, const DSSPage::Options &options, DSSPage *q )
  : object(object), session(session), options(options), q( q )
{
}

DSSPage::~DSSPage()
{
}

void DSSPage::Private::setImageType(DSSImage::ImageVersion version, bool autoStart)
{
  imageContainer->clear();
  DSSImage *image = new DSSImage(object->coordinates(), Angle::degrees(object->angularSize()), version, autoStart, !options.optionsAsMenu, !options.optionsAsMenu );
  image->imageClicked().connect([=](const WMouseEvent &e, _n5) {
    WPopupMenu *menu = new WPopupMenu;
    WMenuItem *i = menu->addItem("Open image in a new window");
    i->setLink(image->imageLink());
    i->setLinkTarget(TargetNewWindow);
    i = menu->addItem("Original DSS Image");
    i->setLink(image->dssOriginalLink());
    i->setLinkTarget(TargetNewWindow);
    if(wApp->environment().agentIsWebKit())
      menu->addItem(WString::tr("buttons_invert"))->triggered().connect([=](WMenuItem*, _n5){ image->toggleStyleClass("image-inverse", !image->hasStyleClass("image-inverse")); });
    WPopupMenu *imageTypeSubmenu = WW<WPopupMenu>().css("dialog-popup-submenu");
    for(auto type: DSSImage::versions()) {
      WString itemName = WString::tr(string{"dssimage_version_"} + DSSImage::imageVersion(type));
      auto typeItem = imageTypeSubmenu->addItem(itemName);
      if(type == version)
        typeItem->addStyleClass("disabled");
      else
        typeItem->triggered().connect([=](WMenuItem*, _n5){
        setImageType(type, true);
      });
      spLog("notice") << "added image type submenu: " << itemName << " " << type;
    }
    menu->addMenu("Change Type...", imageTypeSubmenu);

    menu->popup(e);
  });
  image->failed().connect([=](_n6) mutable {
    if(nextDSSTypeIndex+1 > imageVersions.size())
      return;
    setImageType(imageVersions[nextDSSTypeIndex++], true);
  });
  imageContainer->addWidget(image);
  if(!options.optionsAsMenu) {
    for(int index=0; index<typeModel->rowCount(); index++)
      if(boost::any_cast<DSSImage::ImageVersion>(typeModel->item(index)->data()) == version)
        typeCombo->setCurrentIndex(index);
  }
}

DSSPage::Options DSSPage::Options::embedded(bool autoload)
{
  DSSPage::Options options;
  options.autoStart = autoload;
  options.setPath = false;
  options.showClose = false;
  options.showTitle = false;
  options.optionsAsMenu = true;
  return options;
}

DSSPage::Options DSSPage::Options::standalone(function<void()> runOnClose)
{
  DSSPage::Options options;
  options.runOnClose = runOnClose;
  return options;
}


DSSPage::DSSPage(const NgcObjectPtr &object, Session &session, const DSSPage::Options &options, WContainerWidget *parent )
  : WContainerWidget(parent), d( object, session, options, this )
{
  Dbo::Transaction t(session);
  if(options.setPath) {
    wApp->setInternalPath(DSSPage::internalPath(object, t));
    wApp->setInternalPathValid(true);
  }
  d->imageVersions = {DSSImage::poss2ukstu_red, DSSImage::poss2ukstu_blue, DSSImage::poss1_red, DSSImage::poss1_blue, DSSImage::phase2_gsc2};
  if(object->type() == NgcObject::NebGx || object->type() == NgcObject::NebGx)
    d->imageVersions = {DSSImage::poss2ukstu_blue, DSSImage::poss1_blue, DSSImage::poss2ukstu_red, DSSImage::poss1_red, DSSImage::phase2_gsc2};
  d->imageContainer = WW<WContainerWidget>();
  if(options.showTitle) {
    WString namesJoined = Utils::htmlEncode(WString::fromUTF8( boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object), ", ") ));
    addWidget(new WText{WString("<h4>{1}</h4>").arg(namesJoined)});
  }
  d->typeCombo = WW<WComboBox>().setMargin(10, Wt::Right);
  d->typeModel = new WStandardItemModel(d->typeCombo);
  d->typeCombo->setModel(d->typeModel);

  for(auto type: DSSImage::versions()) {
    auto item = new WStandardItem(WString::tr(string{"dssimage_version_"} + DSSImage::imageVersion(type)));
    item->setData(type);
    d->typeModel->appendRow(item);
  }
  d->typeCombo->activated().connect([=](int index, _n5){
    d->setImageType(boost::any_cast<DSSImage::ImageVersion>(d->typeModel->item(index)->data()), true);
  });

  WPushButton *invertButton = WW<WPushButton>(WString::tr("buttons_invert")).css("btn btn-inverse")
    .onClick([=](WMouseEvent) { d->imageContainer->toggleStyleClass("image-inverse", !d->imageContainer->hasStyleClass("image-inverse")); } )
    .setEnabled(wApp->environment().agentIsWebKit()
  );


  d->toolbar = WW<WContainerWidget>().css("form-inline")
      .add(WW<WLabel>(WString::tr("dssimage_version_label")).setMargin(10, Wt::Right))
      .add(d->typeCombo)
      .add(invertButton);
  if(options.showClose)
    d->toolbar->addWidget(WW<WPushButton>(WString::tr("buttons_close")).css("btn btn-danger pull-right").onClick([=](WMouseEvent){
      options.runOnClose();
    }));

  addWidget(d->toolbar);
  d->toolbar->setHidden(options.optionsAsMenu);

  addWidget(d->imageContainer);
  d->setImageType(d->imageVersions[0], options.autoStart);
  WTemplate *copyright = new WTemplate(R"(
                                       <small>
                                       image copyright:
                                       <a href="http://archive.stsci.edu/dss/" target="_BLANK">The STScI Digitized Sky Survey</a>
                                       <a href="http://archive.stsci.edu/dss/acknowledging.html" target="_BLANK">(acknowledgment)</a>
                                       </small>
                                       )");
  copyright->addStyleClass("pull-left");
  addWidget(copyright);
}

string DSSPage::internalPath( const Dbo::ptr< NgcObject > &object, Dbo::Transaction &transaction )
{
  return format("/dss/%x/%s")
    % object.id()
    % ::Utils::sanitizeForURL(boost::algorithm::join(NgcObject::namesByCatalogueImportance(transaction, object), "-"));
}

