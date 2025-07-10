#pragma once
#include "Engine.h"
#include "Application.h"
#include "ArchiveInitializer.h"
#include "ErrorHandler.h"

Application* CreateApplication();

InitParams parseArgs(int argc, char* argv[])
{
    InitParams params;

    // Iterate over the arguments
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-e") == 0 && i + 1 < argc)
        {
            params.projectDir = argv[i + 1];
            ++i; // Skip the next argument since it's part of the current flag
        }
        else if (strcmp(argv[i], "-ec") == 0 && i + 1 < argc)
        {
            params.projectDir = argv[i + 1];
            params.loadExistingProject = true;
            ++i; // Skip the next argument since it's part of the current flag
        }

        else if (strcmp(argv[i], "-ov") == 0)
        {
            params.overwriteExisting = true;
        }

        else if (strcmp(argv[i], "-ts") == 0)
        {
            params.templateScene = true;
        }

        else if (strcmp(argv[i], "-ss") == 0)
        {
            params.startSimulationOnStartup = true;
        }

        else if (strcmp(argv[i], "-fs") == 0)
        {
            params.fullScreen = true;
        }
    }

    return params;
}

int sgeEntry(int argc, char* argv[])
{
#ifndef SGE_DEBUG
    try
    {
#endif //SGE_DEBUG

#if SGE_SHIPPING
        InitParams initParams;
        initParams.loadExistingProject = true;
        initParams.startSimulationOnStartup = true;
        initParams.shipping = true;
        initParams.projectDir = "./data/";
#else
        InitParams initParams = parseArgs(argc, argv);
#endif // SGE_SHIPPING

        ArchiveInitializer::init();

        auto engine = Engine::get();
        if (!engine->init(initParams)) 
            return 1;

        auto app = CreateApplication();
        app->start();

        engine->run(app);

        app->close();
        engine->close();
#ifndef SGE_DEBUG

    }
    catch (const std::runtime_error& e)
    {
        return ErrorHandler::handle(e);
    }
#endif //SGE_DEBUG
    return 0;
}

int main(int argc, char* argv[])
{ 
	return sgeEntry(argc, argv);
}