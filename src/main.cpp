/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
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
#include "quitresource.h"
#include <Wt/Http/Response>
#include "webservice/activesessionsresource.h"
#include "webservice/dbo_restresource.h"
#include "urls.h"
#include "settings.h"

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
	if(!Settings::instance().style_css_path().is_set()) {
	  server.addResource(new WMemoryResource("text/css", SkyPlannerStyle::css()), "/skyplanner_style.css");
	  server.log("notice") << "Missing skyplanner stylecsspath, using default WMemoryResource";
	}
	
	auto admin_password = Settings::instance().admin_password();
        if(admin_password) {
            server.addResource(new QuitResource(*admin_password), "/quit-forced");
            server.addResource(new QuitResource(*admin_password, [] { 
              std::cerr << "quit-resource called with waiting mode, active sessions: " << activeSessions.size() << std::endl;
              return activeSessions.size() == 0; 
            }), "/quit-waiting");
            server.addResource(new ActiveSessionsResource(activeSessions, *admin_password), "/active-sessions");
        }
        
	server.addResource((new DboRestsResource<NgcObject>())->handleAll()->handleById(), "/rest/skyobjects");
	server.addResource((new DboRestsResource<Catalogue>())->handleAll()->handleById(), "/rest/catalogues");
	server.addResource((new DboRestsResource<NebulaDenomination>())->handleAll()->handleById(), "/rest/skyobjects-names");
        
        auto addStaticResource = [&server] (const boost::filesystem::path &relPath, const std::string &deployPath) {
          auto resource_path = boost::filesystem::path(SHARED_PREFIX) / relPath;
          server.addResource(new WFileResource(resource_path.string()), deployPath);
        };
        addStaticResource("logo_350.png", "/skyplanner_logo.png");
        addStaticResource("loading-64.png", URLs::loading_indicator);

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
