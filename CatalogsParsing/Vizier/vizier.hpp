#ifndef VIZIER_H
#define VIZIER_H

#include <memory>
#include <boost/program_options.hpp>
#include <string>
#include "Models"
#include <functional>
#include <map>

class Session;

class Vizier {
public:
  struct FindOrCreateCatalogue {
    std::function<void(Wt::Dbo::Query<CataloguePtr> &)> findCatalogue;
    std::function<Catalogue*()> createCatalogue;
  };

  struct Column {
    std::string name;
    std::string unit;
    std::string value;
    Angle degrees() const;
    Angle arcMinutes() const;
    double asDouble() const;
  };
  typedef std::map<std::string,Column> Row;

  explicit Vizier(const boost::program_options::variables_map &variables_map);
  ~Vizier();
  static boost::program_options::options_description vizierProgramOptions();
  Session &session() const;
  Wt::Dbo::Transaction &transaction() const;
  std::vector<Row> rows() const;
  CataloguePtr findOrCreateCatalogue(FindOrCreateCatalogue f, Wt::Dbo::Transaction &t) const;
private:
  class Private;
  friend class Private;
  const std::unique_ptr<Private> d;
};

#endif // VIZIER_H


