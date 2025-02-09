// CRO-MAG RALLY ENTRY POINT
// (C) 2022 Iliyas Jorio
// This file is part of Cro-MagRally. https://github.com/jorio/cromagrally

#include "Pomme.h"
#include "PommeInit.h"
#include "PommeFiles.h"

#include <iostream>
#include <cstring>
#include <thread>

#if __APPLE__
#ifndef WATCH
#include <libproc.h>
#include <unistd.h>
#endif
#endif

extern "C"
{
#ifndef WATCH
    #include <SDL.h>
#endif

#ifdef TINYGL
    #include "GL/gl.h"
    #include "zbuffer.h"
#endif

	#include "game.h"
	#include "version.h"

#ifndef WATCH
	SDL_Window* gSDLWindow = nullptr;
#endif
	FSSpec gDataSpec;
	CommandLineOptions gCommandLine;
	int gCurrentAntialiasingLevel;

#ifdef TINYGL
    ZBuffer* gFrameBuffer = NULL;
#ifdef WATCH
    PIXEL* gPixels = NULL;
    bool gPixelsReady = false;
    GLint gPitch = NULL;
    std::string gDataPath = "";
    Byte gInputFlags = 0;
    bool gInputDecel = false;
    bool gInputBrake = false;
    float gInputSteer = 0;
    bool gCanSteer = false;
    bool gCrashThud = false;
#else
    SDL_Surface* gSurface = NULL;
#endif
#endif

#if 0 //_WIN32
	// Tell Windows graphics driver that we prefer running on a dedicated GPU if available
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
#endif

	int GameMain(void);
}

static fs::path FindGameData()
{
	fs::path dataPath;

#if __APPLE__
#ifdef WATCH
    dataPath = gDataPath;
#else
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];

    pid_t pid = getpid();
    int ret = proc_pidpath(pid, pathbuf, sizeof(pathbuf));
    if (ret <= 0)
    {
        throw std::runtime_error(std::string(__func__) + ": proc_pidpath failed: " + std::string(strerror(errno)));
    }

    dataPath = pathbuf;
    dataPath = dataPath.parent_path().parent_path() / "Resources";
#endif
#else
	dataPath = "Data";
#endif

	dataPath = dataPath.lexically_normal();

	// Set data spec -- Lets the game know where to find its asset files
	gDataSpec = Pomme::Files::HostPathToFSSpec(dataPath / "Skeletons");

	// Use application resource file
	auto applicationSpec = Pomme::Files::HostPathToFSSpec(dataPath / "System" / "Application");
	short resFileRefNum = FSpOpenResFile(&applicationSpec, fsRdPerm);

	if (resFileRefNum == -1)
	{
		throw std::runtime_error("Couldn't find the Data folder.");
	}

	UseResFile(resFileRefNum);

	return dataPath;
}

static void ParseCommandLine(int argc, char** argv)
{
	memset(&gCommandLine, 0, sizeof(gCommandLine));
	gCommandLine.vsync = 1;

	for (int i = 1; i < argc; i++)
	{
		std::string argument = argv[i];

		if (argument == "--track")
		{
			GAME_ASSERT_MESSAGE(i + 1 < argc, "practice track # unspecified");
			gCommandLine.bootToTrack = atoi(argv[i + 1]);
			i += 1;
		}
		else if (argument == "--car")
		{
			GAME_ASSERT_MESSAGE(i + 1 < argc, "car # unspecified");
			gCommandLine.car = atoi(argv[i + 1]);
			i += 1;
		}
		else if (argument == "--stats")
			gDebugMode = 1;
		else if (argument == "--no-vsync")
			gCommandLine.vsync = 0;
		else if (argument == "--vsync")
			gCommandLine.vsync = 1;
		else if (argument == "--adaptive-vsync")
			gCommandLine.vsync = -1;
#if 0
		else if (argument == "--fullscreen-resolution")
		{
			GAME_ASSERT_MESSAGE(i + 2 < argc, "fullscreen width & height unspecified");
			gCommandLine.fullscreenWidth = atoi(argv[i + 1]);
			gCommandLine.fullscreenHeight = atoi(argv[i + 2]);
			i += 2;
		}
		else if (argument == "--fullscreen-refresh-rate")
		{
			GAME_ASSERT_MESSAGE(i + 1 < argc, "fullscreen refresh rate unspecified");
			gCommandLine.fullscreenRefreshRate = atoi(argv[i + 1]);
			i += 1;
		}
#endif
	}
}

static void GetInitialWindowSize(int display, int& width, int& height)
{
	const float aspectRatio = 16.0f / 9.0f;
	const float screenCoverage = 2.0f / 3.0f;

#ifndef WATCH
	SDL_Rect displayBounds = { .x = 0, .y = 0, .w = 640, .h = 480 };
	SDL_GetDisplayUsableBounds(display, &displayBounds);

	if (displayBounds.w > displayBounds.h)
	{
		width	= displayBounds.h * screenCoverage * aspectRatio;
		height	= displayBounds.h * screenCoverage;
	}
	else
	{
		width	= displayBounds.w * screenCoverage;
		height	= displayBounds.w * screenCoverage / aspectRatio;
	}
#endif
}

static void Boot()
{
	// Start our "machine"
	Pomme::Init();

	// Load game prefs before starting
	InitDefaultPrefs();
	LoadPrefs();

retryVideo:
	// Initialize SDL video subsystem
#ifndef WATCH
	if (0 != SDL_Init(SDL_INIT_VIDEO))
	{
		throw std::runtime_error("Couldn't initialize SDL video subsystem.");
	}
#endif

	// Create window
#ifndef TINYGL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	gCurrentAntialiasingLevel = gGamePrefs.antialiasingLevel;
	if (gCurrentAntialiasingLevel != 0)
	{
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1 << gCurrentAntialiasingLevel);
	}
#endif

#ifndef WATCH
	int display = gGamePrefs.monitorNum;
	if (display >= SDL_GetNumVideoDisplays())
	{
		display = 0;
	}
#endif

#ifdef PORTRAIT
    int initialWidth = 320;
    int initialHeight = 390;
#else
    int initialWidth = 640;
    int initialHeight = 480;
#endif
	//GetInitialWindowSize(display, initialWidth, initialHeight);

#ifdef TINYGL
#ifdef WATCH
    gPitch = initialWidth * 4;
#else
    int windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
#endif
#else
    int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
#endif
    
#ifndef WATCH
	gSDLWindow = SDL_CreateWindow(
			"Cro-Mag Rally " PROJECT_VERSION,
			SDL_WINDOWPOS_UNDEFINED_DISPLAY(display),
			SDL_WINDOWPOS_UNDEFINED_DISPLAY(display),
			initialWidth,
			initialHeight,
			windowFlags);

	if (!gSDLWindow)
	{
		if (gCurrentAntialiasingLevel != 0)
		{
			printf("Couldn't create SDL window with the requested MSAA level. Retrying without MSAA...\n");

			// retry without MSAA
			gGamePrefs.antialiasingLevel = 0;
			SDL_QuitSubSystem(SDL_INIT_VIDEO);
			goto retryVideo;
		}
		else
		{
			throw std::runtime_error("Couldn't create SDL window.");
		}
	}
#endif

#ifdef TINYGL
#ifdef WATCH
    gPixels = (PIXEL *) malloc(initialWidth * initialHeight * sizeof(PIXEL));
    gPixelsReady = true;
#else
    gSurface = SDL_GetWindowSurface(gSDLWindow);
#endif
    gFrameBuffer = ZB_open(initialWidth, initialHeight, ZB_MODE_RGBA, 0);
    glInit(gFrameBuffer);
#endif
    
	// Find path to game data folder
	fs::path dataPath = FindGameData();

	// Init joystick subsystem
#ifndef WATCH
	{
		SDL_Init(SDL_INIT_GAMECONTROLLER);
		auto gamecontrollerdbPath8 = (dataPath / "System" / "gamecontrollerdb.txt").u8string();
		if (-1 == SDL_GameControllerAddMappingsFromFile((const char*)gamecontrollerdbPath8.c_str()))
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Cro-Mag Rally", "Couldn't load gamecontrollerdb.txt!", gSDLWindow);
		}
	}
#endif
}

static void Shutdown()
{
	Pomme::Shutdown();
#ifndef WATCH
	SDL_Quit();
#endif
}

#ifdef WATCH
void GameThread(std::string dataPath) {
    gDataPath = dataPath;
    try
    {
        Boot();
        GameMain();
    }
    catch (std::exception& ex)
    {
        std::string finalErrorMessage = ex.what();
        std::cerr << "Uncaught exception in game thread: " << finalErrorMessage << "\n";
    }
    catch (...) {}
}

void GetAudio(float *const *stream, int len) {
    Pomme::GetAudio(stream, len);
}
#else
int main(int argc, char** argv)
{
	int				returnCode				= 0;
	std::string		finalErrorMessage		= "";
	bool			showFinalErrorMessage	= false;

	try
	{
		ParseCommandLine(argc, argv);
		Boot();
		returnCode = GameMain();
	}
	catch (Pomme::QuitRequest&)
	{
		// no-op, the game may throw this exception to shut us down cleanly
	}
#if !(_DEBUG)
	// In release builds, catch anything that might be thrown by GameMain
	// so we can show an error dialog to the user.
	catch (std::exception& ex)		// Last-resort catch
	{
		returnCode = 1;
		finalErrorMessage = ex.what();
		showFinalErrorMessage = true;
	}
	catch (...)						// Last-resort catch
	{
		returnCode = 1;
		finalErrorMessage = "unknown";
		showFinalErrorMessage = true;
	}
#endif

	Shutdown();

	if (showFinalErrorMessage)
	{
		std::cerr << "Uncaught exception: " << finalErrorMessage << "\n";
		SDL_ShowSimpleMessageBox(0, "Cro-Mag Rally", finalErrorMessage.c_str(), nullptr);
	}

	return returnCode;
}
#endif
