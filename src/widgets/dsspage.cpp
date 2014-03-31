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

void DSSPage::Private::setImageType(DSS::ImageVersion version, const shared_ptr<mutex> &downloadMutex)
{
  imageContainer->clear();

  Dbo::Transaction t(session);
  ViewPort viewPort = ViewPort::findOrCreate(version, object, session.user(), t);
  DSSImage::ImageOptions dssImageOptions{viewPort.coordinates(), viewPort.angularSize(), viewPort.imageVersion(), options.imageSize};
  dssImageOptions.originalCoordinates.coordinates = object->coordinates();
  dssImageOptions.originalCoordinates.size = ViewPort::defaultAngle(object, t);


  DSSImage *image = new DSSImage(dssImageOptions, downloadMutex, !options.optionsAsMenu, !options.optionsAsMenu );

  dssImageOptions.onViewPortChanged = [=](const Coordinates::Equatorial &coordinates, const Angle &angularsize) {
    Dbo::Transaction t(session);
    ViewPort::save(coordinates, angularsize, image->imageVersion() , object, session.user(), t);
  };


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
    for(auto type: DSS::versions()) {
      WString itemName = WString::tr(string{"dssimage_version_"} + DSS::imageVersion(type));
      auto typeItem = imageTypeSubmenu->addItem(itemName);
      if(type == viewPort.imageVersion() )
        typeItem->addStyleClass("disabled");
      else
        typeItem->triggered().connect([=](WMenuItem*, _n5){
        Dbo::Transaction t(session);
        ViewPort::setImageVersion(type, object, session.user(), t);
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
    Dbo::Transaction t(session);
    auto nextVersion = imageVersions[nextDSSTypeIndex++];
    ViewPort::setImageVersion(nextVersion, object, session.user(), t);
    setImageType(nextVersion, downloadMutex);
  });
  imageContainer->addWidget(image);
  if(!options.optionsAsMenu) {
    for(int index=0; index<typeModel->rowCount(); index++)
      if(boost::any_cast<DSS::ImageVersion>(typeModel->item(index)->data()) == viewPort.imageVersion() )
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
  d->imageVersions = {DSS::poss2ukstu_red, DSS::poss2ukstu_blue, DSS::poss1_red, DSS::poss1_blue, DSS::phase2_gsc2};
  if(object->type() == NgcObject::NebGx || object->type() == NgcObject::NebGx)
    d->imageVersions = {DSS::poss2ukstu_blue, DSS::poss1_blue, DSS::poss2ukstu_red, DSS::poss1_red, DSS::phase2_gsc2};
  d->imageContainer = WW<WContainerWidget>();
  if(options.showTitle) {
    WString namesJoined = Utils::htmlEncode(WString::fromUTF8( boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object), ", ") ));
    addWidget(new WText{WString("<h4>{1}</h4>").arg(namesJoined)});
  }
  d->typeCombo = WW<WComboBox>().setMargin(10, Wt::Right);
  d->typeModel = new WStandardItemModel(d->typeCombo);
  d->typeCombo->setModel(d->typeModel);

  for(auto type: DSS::versions()) {
    auto item = new WStandardItem(WString::tr(string{"dssimage_version_"} + DSS::imageVersion(type)));
    item->setData(type);
    d->typeModel->appendRow(item);
  }
  d->typeCombo->activated().connect([=](int index, _n5){
    Dbo::Transaction t(d->session);
    auto type = boost::any_cast<DSS::ImageVersion>(d->typeModel->item(index)->data());
    ViewPort::setImageVersion(type, object, d->session.user(), t);
    d->setImageType(type, options.downloadMutex);
  });

  WPushButton *invertButton = WW<WPushButton>(WString::tr("buttons_invert")).css("btn btn-inverse")
    .onClick([=](WMouseEvent) { toggleInvert(); } )
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

void DSSPage::toggleInvert()
{
  d->imageContainer->toggleStyleClass("image-inverse", !d->imageContainer->hasStyleClass("image-inverse")); 
}

string DSSPage::internalPath( const Dbo::ptr< NgcObject > &object, Dbo::Transaction &transaction )
{
  return format("/dss/%x/%s")
    % object.id()
    % ::Utils::sanitizeForURL(boost::algorithm::join(NgcObject::namesByCatalogueImportance(transaction, object), "-"));
}

