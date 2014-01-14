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
    container->addWidget(new WText{WString("<strong>{1}</strong>: {2}")
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
