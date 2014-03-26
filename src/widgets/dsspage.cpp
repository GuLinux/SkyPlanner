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
using namespace Wt;
using namespace WtCommons;
using namespace std;



DSSPage::Private::Private(const NgcObjectPtr &object, Session &session, DSSPage *q )
  : object(object), session(session), q( q )
{
}

DSSPage::~DSSPage()
{
}

void DSSPage::Private::setImageType(DSSImage::ImageVersion version, bool autoStart)
{
  imageContainer->clear();
  DSSImage *image = new DSSImage(object->coordinates(), Angle::degrees(object->angularSize()), version );
  image->failed().connect([=](_n6) mutable {
    if(nextDSSTypeIndex+1 > imageVersions.size())
      return;
    setImageType(imageVersions[nextDSSTypeIndex++], true);
  });
  imageContainer->addWidget(image);
  for(int index=0; index<typeModel->rowCount(); index++)
    if(boost::any_cast<DSSImage::ImageVersion>(typeModel->item(index)->data()) == version)
      typeCombo->setCurrentIndex(index);
}

DSSPage::DSSPage(const NgcObjectPtr &object, Session &session, const DSSPage::Options &options, WContainerWidget *parent )
  : WContainerWidget(parent), d( object, session, this )
{
  Dbo::Transaction t(session);
  wApp->setInternalPath(DSSPage::internalPath(object, t));
  wApp->setInternalPathValid(true);
  d->imageVersions = {DSSImage::poss2ukstu_red, DSSImage::poss2ukstu_blue, DSSImage::poss1_red, DSSImage::poss1_blue, DSSImage::phase2_gsc2};
  if(object->type() == NgcObject::NebGx || object->type() == NgcObject::NebGx)
    d->imageVersions = {DSSImage::poss2ukstu_blue, DSSImage::poss1_blue, DSSImage::poss2ukstu_red, DSSImage::poss1_red, DSSImage::phase2_gsc2};
  d->imageContainer = WW<WContainerWidget>();
  WString namesJoined = Utils::htmlEncode(WString::fromUTF8( boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object), ", ") ));
  addWidget(new WText{WString("<h4>{1}</h4>").arg(namesJoined)});
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

 
  WContainerWidget *toolbar = WW<WContainerWidget>().css("form-inline")
      .add(WW<WLabel>(WString::tr("dssimage_version_label")).setMargin(10, Wt::Right))
      .add(d->typeCombo)
      .add(invertButton);
  addWidget(toolbar);
  if(options.showClose)
    toolbar->addWidget(WW<WPushButton>(WString::tr("buttons_close")).css("btn btn-danger pull-right").onClick([=](WMouseEvent){
      options.runOnClose();
    }));
  addWidget(d->imageContainer);
  d->setImageType(d->imageVersions[0], options.autoStart);
  addWidget(WW<WContainerWidget>().css("pull-left")
        .add(new WText{"Images Copyright: "})
        .add(WW<WAnchor>("http://archive.stsci.edu/dss/", "The STScI Digitized Sky Survey").setTarget(Wt::TargetNewWindow))
        .add(WW<WAnchor>("http://archive.stsci.edu/dss/acknowledging.html", " (Acknowledgment)").setTarget(Wt::TargetNewWindow))
  );
}

string DSSPage::internalPath( const Dbo::ptr< NgcObject > &object, Dbo::Transaction &transaction )
{
  return format("/dss/%x/%s")
    % object.id()
    % ::Utils::sanitizeForURL(boost::algorithm::join(NgcObject::namesByCatalogueImportance(transaction, object), "-"));
}

