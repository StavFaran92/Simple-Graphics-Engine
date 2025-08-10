#include "core/Window.h"

#include <GL/glew.h>
#include "SDL2/SDL.h"


#include "core/Logger.h"
#include "core/Engine.h"

void APIENTRY MyDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* message, const void* userParam) {
	logError("OpenGL Debug: {]", std::string(message));
}

Window::Window()
{
}

int Window::init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		logError("SDL could not initialize! SDL Error: {}", SDL_GetError());
		return false;
	}

	//Use OpenGL 4.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN);

	m_width = SCREEN_WIDTH;
	m_height = SCREEN_HEIGHT;

	if (Engine::get()->getInitParams().fullScreen || Engine::get()->getInitParams().shipping)
	{
		window_flags = (SDL_WindowFlags)(window_flags | SDL_WINDOW_FULLSCREEN_DESKTOP);

		SDL_Rect displayBounds;
		if (SDL_GetDisplayBounds(0, &displayBounds) == 0)
		{
			m_width = displayBounds.w;
			m_height = displayBounds.h;
		}
	}

	//Create window
	m_mainWindow = SDL_CreateWindow("SGE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, window_flags);
	if (!m_mainWindow)
	{
		logError("Window could not be created! SDL Error: {}", SDL_GetError());
		return false;
	}
	
	//Create context
	m_glContext = SDL_GL_CreateContext(m_mainWindow);
	if (!m_glContext)
	{
		logError("OpenGL context could not be created! SDL Error: {}", SDL_GetError());
		return false;
	}

	if (SDL_GL_MakeCurrent(m_mainWindow, m_glContext) != 0)
	{
		logError("Make current context failed! SDL Error: {}", SDL_GetError());
		return false;
	}
	
	//Initialize GLEW
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK)
	{
		logError("Error initializing GLEW! {}", glewGetErrorString(glewError));
		return false;
	}

	//glEnable(GL_DEBUG_OUTPUT);
	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	//glDebugMessageCallback(MyDebugCallback, nullptr);

	//Use Vsync
	if (SDL_GL_SetSwapInterval(1) != 0)
	{
		logError("Warning: Unable to set VSync! SDL Error: {}", SDL_GetError());
		return false;
	}

	m_halfWidth = m_width / 2.f;
	m_halfHeight = m_height / 2.f;
		
        logInfo("SDL has initialized successfully.");

        return true;
}

void Window::resize(int width, int height)
{
        m_width = width;
        m_height = height;
        m_halfWidth = m_width / 2.f;
        m_halfHeight = m_height / 2.f;
}

void Window::close()
{

	// Delete context
	SDL_GL_DeleteContext(m_glContext);

	//Destroy window
	SDL_DestroyWindow(m_mainWindow);

	//Quit SDL subsystems
	SDL_Quit();

	m_mainWindow = NULL;
}

inline void Window::SwapBuffer() 
{ 
	SDL_GL_SwapWindow(m_mainWindow); 
}

void Window::update()
{ 
	//if (m_isMouseLocked)
	//	SDL_WarpMouseInWindow(m_mainWindow, m_halfWidth, m_halfHeight); 
}

void Window::lockMouse()
{
	SDL_SetRelativeMouseMode(SDL_TRUE);
}

void Window::unlockMouse()
{
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void* Window::GetNativeWindow()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_mainWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;
	return hwnd;
}

Window::~Window() {
	close();
}