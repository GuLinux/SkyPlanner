#include <iostream>
#include <Wt/WServer>
#include <Wt/WMemoryResource>
#include <signal.h>
#include "ngcresource.h"
#include "astroplanner.h"
#include "style.css.h"
#include "session.h"
using namespace std;
using namespace Wt;


WApplication *createAstroPlanner(const WEnvironment &env)
{
   return new AstroPlanner(env);
}

int main(int argc, char **argv) {
    try {
        WServer server(argv[0]);
        server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);
        server.addResource(new NgcResource, "/ngc");
	server.addResource(new WMemoryResource("text/css", styleCss()), "/astroplanner_style.css");
        server.addEntryPoint(Wt::Application, createAstroPlanner, "/");
        Session::configureAuth();
        if (server.start()) {
            int sig = WServer::waitForShutdown(argv[0]);
            std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
            server.stop();
            if (sig == SIGHUP)
                WServer::restart(argc, argv, environ);
        }
    } catch (WServer::Exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    } catch (std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }
}
