#include <iostream>
#include "models/Models"
#include "session.h"
#include <boost/program_options.hpp>
#include <vector>
#include "dss.h"
#include <widgets/dssimage.h>
#include <utils/curl.h>
#include <utils/utils.h>
#include <utils/format.h>

using namespace std;
using namespace Wt;

namespace po = boost::program_options;

template<typename T>
bool check_option(const po::variables_map &vm, string name, const vector<T> &values = {}) {
  if(vm.count(name) == 0) {
     cerr << "Error! parameter " << name << " missing." << endl;
     return false;
  }
  if(values.size()>0) {
    T value = vm[name].as<T>();
    if(find(begin(values), end(values), value) == end(values)) {
      cerr << "Error! value " << value << " not allowed for option " << name << ".\nAllowed values:";
      for(auto v: values)
	cerr << " " << v;
      cerr << endl;
      return false;
    }
  }
  return true;
}

int main(int argc, char **argv) {
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("db-connection", po::value<string>(), "database connection string")
      ("db-type", po::value<string>(), "database type (pg, sqlite3)")
      ("outdir", po::value<string>(), "output directory")
  ;
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);    

  if (vm.count("help")) {
      cout << desc << "\n";
      return 1;
  }

  map<string, Session::Provider> dbTypes {
    {"pg", Session::Postgres},
    {"sqlite3", Session::Sqlite3},
  };
  vector<string> dbTypes_keys;
  transform(begin(dbTypes), end(dbTypes), back_inserter(dbTypes_keys), [](const pair<string,Session::Provider> &p){ return p.first; });
  
  if (! check_option<string>(vm, "outdir") || ! check_option<string>(vm, "db-connection") || ! check_option<string>(vm, "db-type", dbTypes_keys)) {
    cerr << desc;
    return 1;
  }
  
  cout << "Dbo connection: " << vm["db-connection"].as<string>() << ", type: " << vm["db-type"].as<string>() << endl;
  string outdir = vm["outdir"].as<string>();

  Session session(vm["db-connection"].as<string>(), dbTypes[vm["db-type"].as<string>()]);
  dbo::Transaction t(session);

  auto objects = session.find<NgcObject>().resultList();
  for(auto object: objects) {
    for(auto dsstype: vector<DSS::ImageVersion>{      
      DSS::poss2ukstu_red,
      DSS::poss2ukstu_blue,
      DSS::poss2ukstu_ir,
      DSS::poss1_red,
      DSS::poss1_blue,
      DSS::quickv,
      DSS::phase2_gsc2,
      DSS::phase2_gsc1,}) {
        ViewPort viewPort = ViewPort::findOrCreate(dsstype, object, {}, t); // TODO: parameters
        DSSImage::ImageOptions dssImageOptions{viewPort.coordinates(), viewPort.angularSize(), viewPort.imageVersion(), DSSImage::Full};
        auto outfile = dssImageOptions.file(outdir);
        if(boost::filesystem::exists(outfile)) {
          cerr << "File already existing: " << outfile << endl;
          continue;
        }
      ofstream out(outfile.string());
      Curl curl{out};
      Scope cleanup([&]{
        out.close();
        if( curl.header("Content-Type") != "image/gif"  || boost::lexical_cast<uint64_t>(curl.header("Content-length")) != boost::filesystem::file_size(outfile) || ! curl.requestOk() || curl.httpResponseCode() != 200 ) {
          cerr << "Error downloading " << dssImageOptions.url() << ": " << curl.lastErrorMessage() << "; content type: " << curl.header("Content-Type") << ", status: " << curl.httpResponseCode() << endl;
	  string suffix = format(".%d.html") % curl.httpResponseCode();
	  auto error_file = outfile;
          boost::filesystem::rename(outfile, error_file.replace_extension(suffix));
        }
      });
      curl.get(dssImageOptions.url());
      // cout << object.id() << "|" << dssImageOptions.url() << "|" << dssImageOptions.file(outdir).string() << endl;
    }
  }
}

// TODO: remove in some way
#include "skyplanner.h"

WLogEntry SkyPlanner::uLog(const string &type) const
{
}
SkyPlanner *SkyPlanner::instance()
{
  return nullptr;
}
