#include <iostream>
#include <Wt/WServer>
#include <Wt/WMemoryResource>
#include <signal.h>
#include "ngcresource.h"
#include "skyplanner.h"
#include "style.css.h"
#include "session.h"
#include "utils/utils.h"
#include "curl/curl.h"

using namespace std;
using namespace Wt;


WApplication *createAstroPlanner(const WEnvironment &env)
{
   return new SkyPlanner(env);
}

int main(int argc, char **argv) {
    curl_global_init(CURL_GLOBAL_ALL);
    Scope cleanup([] {  curl_global_cleanup(); });
    try {
        WServer server(argv[0]);
        server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
        server.addResource(new NgcResource, "/ngc");
	server.addResource(new WMemoryResource("text/css", styleCss()), "/skyplanner_style.css");
        server.addEntryPoint(Wt::Application, createAstroPlanner);
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
