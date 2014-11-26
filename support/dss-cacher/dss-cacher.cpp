#include <iostream>
#include "models/Models"
#include "session.h"
#include <boost/program_options.hpp>
#include <vector>
#include <signal.h>
#include "dss.h"
#include <widgets/dssimage.h>
#include <utils/curl.h>
#include <utils/utils.h>
#include <utils/format.h>
#include <boost/thread.hpp>
#include <GraphicsMagick/Magick++.h>

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

bool keepGoing = true;

void handleInterrupt(int s) {
  keepGoing = false;
  cerr << "Catched interrupt, finishing current downloads and cleaning up..." << endl;
}

struct DSSDownloader {
  string url;
  boost::filesystem::path file;
  string object_id;
  double magnitude;
  void download() const;
  operator string() const { return format("{%s; magnitude: %d; url: %s; file: %s}") % object_id % magnitude % url % file.string(); }
};


void DSSDownloader::download() const
{
  if(boost::filesystem::exists(file)) {
    cerr << "File already existing: " << file << endl;
    return;
  }
  stringstream mem;
  Curl curl{mem};
  Scope cleanup([&]{
    auto received_content_type = curl.header("Content-Type");
    uint64_t received_content_length = 0;
    try {
      received_content_length = boost::lexical_cast<uint64_t>(curl.header("Content-length"));
      Magick::Blob blob(mem.str().data(), mem.str().size());
      Magick::Image image(blob);
      image.write(file.string());
    } catch(const std::exception &e) {
      ofstream out(format("%s.%d.html") % file.string() % curl.httpResponseCode() );
      cerr << "Error downloading " << url << ": " << curl.lastErrorMessage() << "; content type: " << received_content_type << ", status: " << curl.httpResponseCode() << endl;
      out << format(R"(
	<!-- 
	Error details:
	  original url: %s
	  curl response code: %d
	  curl error message: %s
	  expected size: %d
	  actual size: %d
	  exception message: %s
	-->)") % url % curl.httpResponseCode() % curl.lastErrorMessage() % received_content_length % boost::filesystem::file_size(file) % e.what();
      out.close();
    }
  });
  curl.get(url);
}

class thread_pool {
public:
  thread_pool(int max_threads = 1) : max_threads(max_threads) {}
  void run(function<void()> f);
private:
  int max_threads;
  int threads_number = 0;
  boost::mutex mutex;
};

void thread_pool::run(function<void()> f)
{
  while(threads_number > max_threads)
    boost::this_thread::sleep_for(boost::chrono::milliseconds(500));
  boost::unique_lock<boost::mutex> lock(mutex);
  threads_number++;
  boost::thread([=]{
    f();
    boost::unique_lock<boost::mutex> lock(mutex);
    threads_number--;
  });
}


int main(int argc, char **argv) {
  Magick::InitializeMagick(*argv);
  signal(SIGINT, handleInterrupt);
  po::options_description desc("Allowed options");
  desc.add_options()
      ("help", "produce help message")
      ("db-connection", po::value<string>(), "database connection string")
      ("db-type", po::value<string>(), "database type (pg, sqlite3)")
      ("outdir", po::value<string>(), "output directory")
      ("threads", po::value<int>()->default_value(1), "max concurrent downloads (threads), default=1")
      ("minimum-free-space", po::value<int>()->default_value(500), "minimum disk free space (MB), default=500")
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
  
  int minimum_free_space_mb = vm["minimum-free-space"].as<int>();
  auto free_space_mb = [=] {
    return boost::filesystem::space(outdir).available / 1024 / 1024;
  };

  cerr << "Free space (MB): " << free_space_mb() << endl;
  auto objects = session.find<NgcObject>().orderBy("magnitude ASC").resultList();
  int currentObject{0};
  thread_pool pool(vm["threads"].as<int>());
  for(auto object: objects) {
    currentObject++;
    for(auto dsstype: vector<DSS::ImageVersion>{ DSS::poss2ukstu_red, DSS::poss2ukstu_blue, DSS::poss2ukstu_ir, DSS::poss1_red,
      DSS::poss1_blue, DSS::quickv, DSS::phase2_gsc2, DSS::phase2_gsc1,}) {
	if(!keepGoing || free_space_mb() <= minimum_free_space_mb)
	  break;
        ViewPort viewPort = ViewPort::findOrCreate(dsstype, object, {}, t); // TODO: parameters
        DSSImage::ImageOptions dssImageOptions{viewPort.coordinates(), viewPort.angularSize(), viewPort.imageVersion(), DSSImage::Full};
	DSSDownloader image { dssImageOptions.url(), dssImageOptions.file(outdir)};
	string as_string = image;
	cerr << format("%d/%d: %s") % currentObject % objects.size() % as_string << endl;
	pool.run([=]{ image.download(); });
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
