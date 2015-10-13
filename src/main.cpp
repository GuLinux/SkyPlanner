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
#include <Wt/Json/Parser>
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

template<typename T> T json_cast(const Wt::Json::Value &v) { return v; }

void populate_database() {
    Session session;
    Dbo::Transaction t(session);
    if(session.query<long long>("SELECT COUNT(*) FROM objects;") > 0) {
      return;
    }
    WServer::instance()->log("notice") << "*** Database needs to be populated.";
    auto json_file = "skyobjects-min.json";
    auto json_file_path = (boost::filesystem::path(Settings::instance().resources_path()) / json_file).string();
    ifstream json( json_file_path );
    if(!json) {
      WServer::instance()->log("warning") << "Unable to initialize database: " << json_file_path << " could not be read.";
      return;
    }
    stringstream content;
    content << json.rdbuf();
    Wt::Json::Value result;
    try {
    Wt::Json::parse(content.str(), result);
    } catch(const Wt::Json::ParseError &e) {
      WServer::instance()->log("warning") << "Unable to parse " << json_file_path << ": " << e.what();
      return;
    }
    Wt::Json::Array objects = result;
    for(auto object: objects) {
      auto json_object = json_cast<Wt::Json::Object>(object);
      /*
       *         "angular-size": 0.25,
        "declination": 0.449015,
        "extra-data": "",
        "id": 337445.0,
        "magnitude": 14.0,
        "object-id": "MCG+04-02-018",
        "right-ascension": 0.113519,
        "type": 0

        */
      session.execute("INSERT INTO objects(id, object_id, ra, dec, magnitude, angular_size, type, extra_data) VALUES(?,?,?,?,?,?,?,?)")
	.bind(json_cast<long long>(json_object["id"]))
	.bind(json_cast<string>(json_object["object-id"]))
	.bind(json_cast<double>(json_object["right-ascension"]))
	.bind(json_cast<double>(json_object["declination"]))
	.bind(json_cast<double>(json_object["magnitude"]))
	.bind(json_cast<double>(json_object["angular-size"]))
	.bind(json_cast<int>(json_object["type"]))
	.bind(json_cast<string>(json_object["extra-data"]));
    }
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
          auto resource_path = boost::filesystem::path{Settings::instance().resources_path()} / relPath;
          server.addResource(new WFileResource(resource_path.string()), deployPath);
        };
        addStaticResource("logo_350.png", "/skyplanner_logo.png");
        addStaticResource("loading-64.png", URLs::loading_indicator);

	populate_database();
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
