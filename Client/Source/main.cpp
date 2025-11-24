#include "Client/Application.h"
#include <Core/Logging/Logger.h>

int main(int argc, char** argv) {
    try {
        Yamen::Client::Application app;
        app.Run();
        return 0;
    }
    catch (const std::exception& e) {
        YAMEN_CLIENT_CRITICAL("Fatal error: {}", e.what());
        return -1;
    }
}