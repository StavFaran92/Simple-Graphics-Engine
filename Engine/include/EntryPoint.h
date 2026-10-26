#pragma once
#include "core/Engine.h"
#include "core/Application.h"
#include "serialize/ArchiveInitializer.h"
#include "core/ErrorHandler.h"

Application* CreateApplication();

// On dual-GPU machines the drivers read these from the exe and run it on the discrete GPU instead of the integrated one
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

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

        else if (strcmp(argv[i], "-td") == 0)
        {
            params.tempDir = true;
        }

        else if (strcmp(argv[i], "-capture") == 0 && i + 1 < argc)
        {
            params.capturePath = argv[i + 1];
            ++i;
        }

        else if (strcmp(argv[i], "-log") == 0 && i + 1 < argc)
        {
            params.logPath = argv[i + 1];
            ++i;
        }

        else if (strcmp(argv[i], "-frames") == 0 && i + 1 < argc)
        {
            params.captureFrames = atoi(argv[i + 1]);
            ++i;
        }
    }

    // Asking for a capture without a frame count waits long enough for the scene to load
    if ((!params.capturePath.empty() || !params.logPath.empty()) && params.captureFrames <= 0)
        params.captureFrames = 120;

    return params;
}

int sgeEntry(int argc, char* argv[])
{
    // An exception that escapes is logged with its stack (see ErrorHandler::run)
    return ErrorHandler::run([&] {
#if SGE_SHIPPING
        InitParams initParams;
        initParams.loadExistingProject = true;
        initParams.startSimulationOnStartup = true;
        initParams.shipping = true;
        initParams.projectDir = "./data/";
#else
        InitParams initParams = parseArgs(argc, argv);

#ifdef SGE_PROJECT_LAUNCHER
        // No project on the command line: the app lets the user pick one first
        if (initParams.projectDir.empty() && !initParams.tempDir && initParams.captureFrames <= 0)
        {
            if (!SGE_PROJECT_LAUNCHER(initParams))
                return 0;
        }
#endif
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
        return 0;
    });
}

int main(int argc, char* argv[])
{ 
	return sgeEntry(argc, argv);
}