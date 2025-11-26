#include "Client/Application.h"
#include <Core/Logging/Logger.h>

int main(int argc, char** argv) {
    try {
        Yamen::Client::Application app;
        
        Yamen::Client::EngineConfig config;
        config.WindowTitle = "Yamen Engine - Dev Build";
        config.WindowWidth = 1600;
        config.WindowHeight = 900;
        config.VSync = true;
        config.StartScene = "ECS Scene";

        if (app.Initialize(config)) {
            app.Run();
        }
        return 0;
    }
    catch (const std::exception& e) {
        YAMEN_CLIENT_CRITICAL("Fatal error: {}", e.what());
        return -1;
    }
}