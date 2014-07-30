#ifndef ASTROOBJECTWIDGET_H
#define ASTROOBJECTWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <mutex>
#include "astro_group.h"

class Session;
class Timezone;
class AstroObjectWidget : public Wt::WCompositeWidget {
public:
  explicit AstroObjectWidget(const AstroGroup &astroGroup, Session &session, const std::shared_ptr<std::mutex> &downloadMutex = {}, const std::vector<Wt::WPushButton*> &actionButtons = {}, Wt::WContainerWidget *parent = 0);
  ~AstroObjectWidget();
  void toggleInvert();
  void setDSSVisible(bool visible);
  bool isDSSVisible() const;
  void setCollapsed(bool collapsed);
  bool isCollapsed() const;
  void reload();
private:
  D_PTR;
};

#endif // ASTROOBJECTWIDGET_H


