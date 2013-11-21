#ifndef SESSION_P_H
#define SESSION_P_H
#include "session.h"

class Session::Private {
public:
  Private();
  std::shared_ptr<Wt::Dbo::SqlConnection> connection;
};
#endif

