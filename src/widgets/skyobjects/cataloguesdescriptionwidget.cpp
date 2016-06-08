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
#include "cataloguesdescriptionwidget.h"
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/WContainerWidget>
#include <Wt/WTable>

using namespace Wt;
using namespace std;
CataloguesDescriptionWidget::CataloguesDescriptionWidget(const vector<NgcObject::CatalogueDescription> &cataloguesDescriptions, WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  WContainerWidget *container = new WContainerWidget;
  setImplementation(container);

  container->addWidget(new WText{WString::tr("object_row_cataloguedesc")});
  for(auto d: cataloguesDescriptions)
    container->addWidget(new WText{WString("<strong>{1}</strong>: {2}<br />")
                                         .arg(d.catalogue->name() )
                                         .arg(Utils::htmlEncode( WString::fromUTF8(d.description), Utils::HtmlEncodingFlag::EncodeNewLines )
                                         )
                               });
}


WWidget *CataloguesDescriptionWidget::add(WTable *table, int colSpan, const NgcObjectPtr &object, bool hidden)
{
  auto dbDescriptions = object->descriptions();
  if(!dbDescriptions.empty()) {
    WTableRow *descriptionRow = table->insertRow(table->rowCount());
    WTableCell *descriptionCell = descriptionRow->elementAt(0);
    descriptionCell->setHidden(hidden);
    descriptionCell->setColumnSpan(colSpan);
    descriptionCell->addStyleClass("alert alert-info");
    descriptionCell->addWidget(new CataloguesDescriptionWidget{dbDescriptions});
    return descriptionCell;
  }
  return nullptr;
}
