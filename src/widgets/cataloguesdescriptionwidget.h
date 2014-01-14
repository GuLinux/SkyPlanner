#ifndef CATALOGUESDESCRIPTIONWIDGET_H
#define CATALOGUESDESCRIPTIONWIDGET_H

#include <Wt/WCompositeWidget>
#include "Models"
class CataloguesDescriptionWidget : public Wt::WCompositeWidget
{
public:
  CataloguesDescriptionWidget(const std::vector<NgcObject::CatalogueDescription> &cataloguesDescriptions, Wt::WContainerWidget *parent = 0);
  static void add(Wt::WTable *table, int colSpan, const NgcObjectPtr &object);

  // TODO?
  // static void popupInfo(Wt::WWidget *widget, const NgcObjectPtr &object);
};

#endif // CATALOGUESDESCRIPTIONWIDGET_H
