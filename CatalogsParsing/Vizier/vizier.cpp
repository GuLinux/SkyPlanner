#include "vizier.hpp"
#include "vizier_p.hpp"
#include "session.h"
#include <fstream>
#include <boost/algorithm/string.hpp>

namespace po=boost::program_options;
namespace fs=boost::filesystem;
using namespace std;
using namespace Wt;

#define CATALOGUE_FNAME_OPTION "catalogue-filename"
#define DB_CONN "database-connection"
#define DB_TYPE "database-type"


Angle Vizier::Column::arcMinutes() const
{
  try {
    return Angle::arcMinutes(boost::lexical_cast<double>(value));
  } catch(exception &e) {
    return Angle::degrees(0);
  }
}
Angle Vizier::Column::degrees() const
{
  try {
    return Angle::degrees(boost::lexical_cast<double>(value));
  } catch(exception &e) {
    return Angle::degrees(0);
  }
}

double Vizier::Column::asDouble() const
{
  try {
  return boost::lexical_cast<double>(value);
  } catch(exception &e) {
    return -1;
  }
}


Vizier::Private::Private(Vizier *q): q(q)
{
}

Vizier::Vizier(const po::variables_map &variables_map)
  : dptr(this)
{
  auto checkOption = [=](const string &optionName, function<bool(const po::variable_value &)> isValid, const string &errorMessage){
    if( ! variables_map.count(optionName) || ! isValid(variables_map[optionName]) )
      throw runtime_error(errorMessage);
  };

  checkOption(CATALOGUE_FNAME_OPTION, [=](const po::variable_value &v) { return fs::exists(v.as<string>()); }, "Error! file parameter missing");
  checkOption(DB_TYPE, [=](const po::variable_value &v) { return v.as<string>() == "sqlite3" || v.as<string>() == "postgresql"; }, "Error! database type invalid");
  checkOption(DB_CONN, [=](const po::variable_value &v) { return v.as<string>().size(); }, "Error! Database connection string missing");

  auto provider = variables_map[DB_TYPE].as<string>() == "sqlite3" ? Session::Sqlite3 : Session::Postgres;
  d->session.reset(new Session{variables_map[DB_CONN].as<string>(), provider});

  fs::path file = variables_map[CATALOGUE_FNAME_OPTION].as<string>();
  d->parse( file );
}


Vizier::~Vizier()
{
}

CataloguePtr Vizier::findOrCreateCatalogue(Vizier::FindOrCreateCatalogue f, Wt::Dbo::Transaction &t) const
{
  auto query = t.session().find<Catalogue>();
  f.findCatalogue(query);
  auto catalogue = query.resultValue();
  if(!catalogue)
    catalogue = t.session().add(f.createCatalogue() );
  return catalogue;
}


void Vizier::Private::parse(const boost::filesystem::path &path)
{
  rows.clear();
  ifstream f(path.string());
  string line;
  vector<string> header;
  vector<string> units;
  bool found_delim = false;
  auto trim = [](const string &s) { return boost::algorithm::trim_copy(s); };
  while(f) {
    getline(f, line);
    if(trim(line).empty() || trim(line)[0] == '#')
      continue;
    if(!header.size() ) {
      boost::algorithm::split(header, line, boost::is_any_of("\t"));
      continue;
    }
    if(!units.size() ) {
      boost::algorithm::split(units, line, boost::is_any_of("\t"));
      continue;
    }
    if(!found_delim ) {
      found_delim = true;
      continue;
    }
    vector<string> columns;
    Row row;
    boost::algorithm::split(columns, line, boost::is_any_of("\t"));
    for(int i=0; i<header.size() ; i++) {
      row[trim(header[i])] = {trim(header[i]), trim(units[i]), trim(columns[i])};
    }
    rows.push_back(row);
  }
}

po::options_description Vizier::vizierProgramOptions()
{
  po::options_description desc("Vizier Importer Options");
  desc.add_options()
      ("help", "this help message")
      (CATALOGUE_FNAME_OPTION ",f", po::value<string>(), "catalogue file path")
      (DB_TYPE ",d", po::value<string>(), "database connection type (currently supported: sqlite3, postgresql")
      (DB_CONN ",c", po::value<string>(), "database connection string")
  ;
  return desc;
}

Session &Vizier::session() const
{
  return *d->session;
}

vector<Vizier::Row> Vizier::rows() const
{
  return d->rows;
}
