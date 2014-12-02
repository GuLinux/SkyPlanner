#include <iostream>
#include <Wt/WServer>
#include <Wt/WMemoryResource>
#include <Wt/WFileResource>
#include <signal.h>
#include "skyplanner.h"
#include "style.css.h"
#include "session.h"
#include "utils/utils.h"
#include "curl/curl.h"
#include <boost/filesystem.hpp>
#include <Magick++.h>
#include "Wt-Commons/quitresource.h"
#include <Wt/Http/Response>
#include "webservice/activesessionsresource.h"
#include "webservice/dbo_restresource.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

static vector<SkyPlanner*> activeSessions;

WApplication *newSkyPlanner(const WEnvironment &env)
{
   auto nowString = [] {
       return boost::posix_time::to_iso_string(boost::posix_time::second_clock().local_time() );
   };

   auto newApp = new SkyPlanner(env, [nowString](SkyPlanner *app) {
       activeSessions.erase(std::remove(activeSessions.begin(), activeSessions.end(), app));
       std::cerr << nowString() << " - Ending session: activeSessions=" << activeSessions.size() << std::endl;
   });
   activeSessions.push_back(newApp);
   std::cerr << nowString() << " - Starting new session: activeSessions=" << activeSessions.size() << std::endl;
   return newApp;
}

int main(int argc, char **argv) {
    Magick::InitializeMagick(*argv);
    curl_global_init(CURL_GLOBAL_ALL);
    Scope cleanup([] {  curl_global_cleanup(); });
    try {
        WServer server(argv[0]);
        server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
        string noop;
        if(! server.readConfigurationProperty("style-css-path", noop))
              server.addResource(new WMemoryResource("text/css", SkyPlannerStyle::css()), "/skyplanner_style.css");
        string quitResourcePassword;
        if(server.readConfigurationProperty("quit-password", quitResourcePassword)) {
            server.addResource(new QuitResource(quitResourcePassword), "/quit-forced");
            server.addResource(new QuitResource(quitResourcePassword, [] { return activeSessions.size() == 0; }), "/quit-waiting");
            server.addResource(new ActiveSessionsResource(activeSessions, quitResourcePassword), "/active-sessions");
        }
        
	server.addResource((new DboRestsResource<NgcObject>())->handleAll()->handleById(), "/SkyPlanner/api/skyobjects");

        auto logo_path = boost::filesystem::path(RESOURCES_DIRECTORY) / "logo_350.png";
        server.log("notice") << "Using Logo resource: " << logo_path;
        server.addResource(new WFileResource(logo_path.string()), "/skyplanner_logo.png");
        server.addEntryPoint(Wt::Application, newSkyPlanner);
        Session::configureAuth();
        if (server.start()) {
          int sig = WServer::waitForShutdown(argv[0]);
          std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
          server.stop();
    #ifndef WIN32
          if (sig == SIGHUP)
            WServer::restart(argc, argv, environ);
    #endif
        }
    } catch (WServer::Exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }
}
