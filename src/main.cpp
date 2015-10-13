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
    auto objects_count = session.query<boost::tuple<long long, long long, long long>>
      ("select (select count(id) from objects), (select count(id) from catalogues), (select count(id) from denominations);").resultValue();
    if(get<0>(objects_count) > 0 && get<1>(objects_count) && get<2>(objects_count) > 0) {
      return;
    }
    session.execute("DELETE FROM denominations");
    session.execute("DELETE FROM catalogues");
    session.execute("DELETE FROM objects");
    WServer::instance()->log("notice") << "*** Database needs to be populated.";
    
    auto save_ngcobject = [&session](const Wt::Json::Object &json_object) {
      session.execute("INSERT INTO objects(id, object_id, ra, dec, magnitude, angular_size, type, extra_data) VALUES(?,?,?,?,?,?,?,?)")
        .bind(json_cast<long long>(json_object.at("id")))
        .bind(json_cast<string>(json_object.at("object-id")))
        .bind(json_cast<double>(json_object.at("right-ascension")))
        .bind(json_cast<double>(json_object.at("declination")))
        .bind(json_cast<double>(json_object.at("magnitude")))
        .bind(json_cast<double>(json_object.at("angular-size")))
        .bind(json_cast<int>(json_object.at("type")))
        .bind(json_cast<string>(json_object.at("extra-data")));
    };
    auto save_catalogue = [&session](const Wt::Json::Object &json_object) {
      session.execute("INSERT INTO catalogues(id, version, name, code, priority, search_mode, hidden) VALUES(?, 1, ?, ?, ?, ?, ?)")
        .bind(json_cast<long long>(json_object.at("id")))
        .bind(json_cast<string>(json_object.at("name")))
        .bind(json_cast<string>(json_object.at("code")))
        .bind(json_cast<int>(json_object.at("priority")))
        .bind(json_cast<int>(json_object.at("search-mode")))
        .bind(json_cast<int>(json_object.at("hidden-mode")));
    };
    auto save_denomination = [&session](const Wt::Json::Object &json_object) {
      session.execute("INSERT INTO denominations(id, number, name, comment, other_catalogues, objects_id, catalogues_id) VALUES(?,?,?,?,?,?,?)")
        .bind(json_cast<long long>(json_object.at("id")))
        .bind(json_cast<string>(json_object.at("number")))
        .bind(json_cast<string>(json_object.at("name")))
        .bind(json_cast<string>(json_object.at("comment")))
        .bind(json_cast<string>(json_object.at("other-catalogues")))
        .bind(json_cast<long long>(json_object.at("object")))
        .bind(json_cast<long long>(json_object.at("catalogue")));
    };
    typedef function<void(const Wt::Json::Object &)> SaveObject;
    typedef pair<string, SaveObject> ObjectParser;
    
    for(auto json_parser: vector<ObjectParser>{{"skyobjects-min.json", save_ngcobject}, {"catalogues-min.json", save_catalogue}, {"skyobjects-names-min.json", save_denomination}}) {
      auto json_file_path = (boost::filesystem::path(Settings::instance().resources_path()) / json_parser.first).string();
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
        try {
          json_parser.second( json_cast<Wt::Json::Object>(object) );
        } catch(const std::exception &e) {
          WServer::instance()->log("warning") << "Unable to save objects from " << json_file_path << ": " << e.what();
        }
      }
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
