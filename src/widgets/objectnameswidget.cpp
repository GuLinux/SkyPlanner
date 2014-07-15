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

#include "Models"
#include "objectnameswidget.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/format.h"
#include <Wt/WText>
#include <Wt/Utils>
#include "utils/d_ptr_implementation.h"
#include <Wt/WPopupMenu>
#include <Wt/WAnchor>
#include "session.h"
#include "types.h"
#include <boost/algorithm/string/join.hpp>

using namespace WtCommons;
using namespace Wt;
using namespace std;

ObjectNamesWidget::ObjectNamesWidget( const NgcObjectPtr &object, Session &session, WPopupMenu *popup, RenderType renderType, int limitNames, WContainerWidget *parent )
  : WContainerWidget( parent )
{
  Dbo::Transaction t(session);
  auto names = NgcObject::namesByCatalogueImportance(t, object);
  if(limitNames > 0)
    names.resize(limitNames);
  WString namesJoined = Utils::htmlEncode( WString::fromUTF8( boost::algorithm::join( names, ", " ) ) );

  if( renderType == Printable )
  {
    setInline( true );
    addWidget( new WText {namesJoined} );
    return;
  }

  WAnchor *namesText = WW<WAnchor>( "", namesJoined ).css( "link" );
  addWidget( namesText );

  namesText->clicked().connect( [ = ]( WMouseEvent e ) { popup->popup(e); } );
}


ObjectNamesWidget::~ObjectNamesWidget()
{
}
