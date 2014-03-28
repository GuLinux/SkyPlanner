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

void DSSPage::Private::setImageType(DSSImage::ImageVersion version, const shared_ptr<mutex> &downloadMutex)
{
  imageContainer->clear();

  Angle size = Angle::degrees(object->angularSize());
  double multiplyFactor = 2.0;
  if(size < Angle::arcMinutes(20))
    multiplyFactor = 3.5;
  if(size < Angle::arcMinutes(10))
    multiplyFactor = 4.5;
  if(size < Angle::arcMinutes(5) )
    multiplyFactor = 7.;
  if(size < Angle::arcMinutes(1) )
    multiplyFactor = 90.;
  size = min(Angle::arcMinutes(75.0), size * multiplyFactor);
  size = (size <= Angle::degrees(0)) ? Angle::arcMinutes(75.0) : size; // objects without angular size (-1), showing max possible field...

  
  DSSImage::ImageOptions dssImageOptions{object->coordinates(), size, version, options.imageSize};

  DSSImage *image = new DSSImage(dssImageOptions, downloadMutex, !options.optionsAsMenu, !options.optionsAsMenu );
  dssImage = image;
  image->imageClicked().connect([=](const WMouseEvent &e, _n5) {
    WPopupMenu *menu = new WPopupMenu;
    WMenuItem *i = menu->addItem(WString::tr("dss_open_new_window_menu"));
    i->setLink(image->fullImageLink());
    i->setLinkTarget(TargetNewWindow);
    i = menu->addItem(WString::tr("dss_original_menu"));
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
        setImageType(type, downloadMutex);
      });
    }
    menu->addMenu(WString::tr("dss_change_type_menu"), imageTypeSubmenu);
    menu->addItem(WString::tr("imagecontrol-menu"))->triggered().connect([=](WMenuItem*, _n5){ if(dssImage) dssImage->showImageControls(); });

    menu->popup(e);
  });
  image->failed().connect([=](_n6) mutable {
    dssImage = nullptr;
    if(nextDSSTypeIndex+1 > imageVersions.size())
      return;
    setImageType(imageVersions[nextDSSTypeIndex++], downloadMutex);
  });
  imageContainer->addWidget(image);
  if(!options.optionsAsMenu) {
    for(int index=0; index<typeModel->rowCount(); index++)
      if(boost::any_cast<DSSImage::ImageVersion>(typeModel->item(index)->data()) == version)
        typeCombo->setCurrentIndex(index);
  }
}

DSSPage::Options DSSPage::Options::embedded(const shared_ptr<mutex> &downloadMutex)
{
  DSSPage::Options options;
  options.downloadMutex = downloadMutex;
  options.setPath = false;
  options.showClose = false;
  options.showTitle = false;
  options.optionsAsMenu = true;
  options.imageSize = DSSImage::Mid;
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
    d->setImageType(boost::any_cast<DSSImage::ImageVersion>(d->typeModel->item(index)->data()), options.downloadMutex);
  });

  WPushButton *invertButton = WW<WPushButton>(WString::tr("buttons_invert")).css("btn btn-inverse")
    .onClick([=](WMouseEvent) { d->imageContainer->toggleStyleClass("image-inverse", !d->imageContainer->hasStyleClass("image-inverse")); } )
    .setEnabled(wApp->environment().agentIsWebKit()
  );

  WPushButton *imageControls = WW<WPushButton>(WString::tr("imagecontrol-menu")).onClick([=](WMouseEvent){ if(d->dssImage) d->dssImage->showImageControls(); });


  d->toolbar = WW<WContainerWidget>().css("form-inline")
      .add(WW<WLabel>(WString::tr("dssimage_version_label")).setMargin(10, Wt::Right))
      .add(d->typeCombo)
      .add(invertButton)
      .add(imageControls);
  if(options.showClose)
    d->toolbar->addWidget(WW<WPushButton>(WString::tr("buttons_close")).css("btn btn-danger pull-right").onClick([=](WMouseEvent){
      options.runOnClose();
    }));

  addWidget(d->toolbar);
  d->toolbar->setHidden(options.optionsAsMenu);

  addWidget(d->imageContainer);
  d->setImageType(d->imageVersions[0], options.downloadMutex);
  WTemplate *copyright = new WTemplate(R"(
                                       <small>
                                       image copyright:
                                       <a href="http://archive.stsci.edu/dss/" target="_BLANK">The STScI Digitized Sky Survey</a>
                                       <a href="http://archive.stsci.edu/dss/acknowledging.html" target="_BLANK">(acknowledgment)</a>
                                       </small>
                                       )");
  copyright->addStyleClass("pull-left hidden-print");
  addWidget(copyright);
}

string DSSPage::internalPath( const Dbo::ptr< NgcObject > &object, Dbo::Transaction &transaction )
{
  return format("/dss/%x/%s")
    % object.id()
    % ::Utils::sanitizeForURL(boost::algorithm::join(NgcObject::namesByCatalogueImportance(transaction, object), "-"));
}

