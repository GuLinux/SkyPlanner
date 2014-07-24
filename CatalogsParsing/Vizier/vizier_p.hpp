#ifndef VIZIER_P_H
#define VIZIER_P_H

#include "vizier.hpp"
#include <boost/filesystem.hpp>

class Vizier::Private
{
public:
  Private(Vizier *q);
  void parse(const boost::filesystem::path &path);
  std::unique_ptr<Session> session;
  std::vector<Vizier::Row> rows;
private:
  Vizier *q;
};

#endif // VIZIER_H


