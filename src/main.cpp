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
#include <GraphicsMagick/Magick++.h>
#include "Wt-Commons/quitresource.h"
#include <Wt/Http/Response>

using namespace std;
using namespace Wt;
using namespace WtCommons;

static vector<SkyPlanner*> activeSessions;

class ActiveSessionsResource : public Wt::WResource {
public:
    ActiveSessionsResource(const std::string &password, Wt::WObject *parent = 0) : Wt::WResource(parent), _password(password) {}
    virtual void handleRequest(const Wt::Http::Request& request, Wt::Http::Response& response);

private:
    const std::string _password;
};

void ActiveSessionsResource::handleRequest(const Http::Request &request, Http::Response &response)
{
    auto password = request.getParameter("pwd");
    if(_password.empty() || !password || _password != *password) {
      response.setStatus(403);
      response.out() << "403 Forbidden";
      return;
    }
    response.out() << "Active sessions: " << activeSessions.size() << std::endl;
    for(SkyPlanner *app: activeSessions) {
        SkyPlanner::SessionInfo infos = app->sessionInfo();
        response.out() << "Session " << app->sessionId() << ", started: " << boost::posix_time::to_simple_string(infos.started) << ", ip: "
                       << infos.ipAddress << ", user agent: " << infos.userAgent << ", username: " << infos.username << std::endl;
    }
    response.setStatus(200);
}

WApplication *newSkyPlanner(const WEnvironment &env)
{
   auto newApp = new SkyPlanner(env, [](SkyPlanner *app) {
       activeSessions.erase(std::remove(activeSessions.begin(), activeSessions.end(), app));
       std::cerr << "Ending session: activeSessions=" << activeSessions.size() << std::endl;
   });
   activeSessions.push_back(newApp);
   std::cerr << "Starting new session: activeSessions=" << activeSessions.size() << std::endl;
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
            server.addResource(new ActiveSessionsResource(quitResourcePassword), "/active-sessions");
        }
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
