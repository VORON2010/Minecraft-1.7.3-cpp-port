#include "lwjgl/Display.h"

#include <iostream>
#include <stdexcept>

#include "java/String.h"
#include "lwjgl/GLContext.h"
#include "lwjgl/Mouse.h"
#include "lwjgl/Keyboard.h"

#include "external/SDLException.h"
#include "GLTrace.h"

#include "SDL.h"
#include <glad/glad.h>

namespace lwjgl
{
namespace Display
{

static bool close_requested = false;

static DisplayMode current_display_mode(0, 0);


void setDisplayMode(const DisplayMode &display_mode)
{
	if (!display_mode.isFullscreen())
	{
		SDL_SetWindowSize(GLContext::detail::getWindow(), display_mode.getWidth(), display_mode.getHeight());
	}
	current_display_mode = display_mode;
	setFullscreen(display_mode.isFullscreen());
}

DisplayMode getDisplayMode()
{
	return current_display_mode;
}

void setTitle(const jstring &string)
{
	std::string utf8 = String::toUTF8(string);
	SDL_SetWindowTitle(GLContext::detail::getWindow(), utf8.c_str());
}

void setFullscreen(bool fullscreen)
{
	if (SDL_SetWindowFullscreen(GLContext::detail::getWindow(), fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0))
		throw SDLException();
	
	
	if (fullscreen)
	{
		int w, h;
		int freq, bpp;

		SDL_DisplayMode sdl_mode;
		if (SDL_GetWindowDisplayMode(GLContext::detail::getWindow(), &sdl_mode))
			throw SDLException();

		w = sdl_mode.w;
		h = sdl_mode.h;
		freq = sdl_mode.refresh_rate;
		bpp = SDL_BITSPERPIXEL(sdl_mode.format);

		current_display_mode = DisplayMode(w, h, bpp, freq);
	}
	else
	{
		int w, h;
		SDL_GetWindowSize(GLContext::detail::getWindow(), &w, &h);
		current_display_mode = DisplayMode(w, h);
	}
}

bool isCloseRequested()
{
	return close_requested;
}

bool isVisible()
{
	auto flags = SDL_GetWindowFlags(GLContext::detail::getWindow());
	return (flags & SDL_WINDOW_SHOWN) != 0;
}

bool isActive()
{
	auto flags = SDL_GetWindowFlags(GLContext::detail::getWindow());
	return (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void processMessages()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				close_requested = true;
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
				Mouse::detail::pushEvent(e);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
				Keyboard::detail::pushEvent(e);
				break;
#ifdef __ANDROID__
			case SDL_FINGERDOWN:
			case SDL_FINGERUP:
			case SDL_FINGERMOTION:
			{
				// Basic Android Touch-to-Gamepad translation
				// Left half of screen = D-Pad (WASD)
				// Right half of screen = Mouse Look & Click
				bool isLeftHalf = (e.tfinger.x < 0.5f);
				
				if (isLeftHalf) {
					// Virtual joystick for WASD
					SDL_Event keyEvent;
					keyEvent.type = (e.type == SDL_FINGERUP) ? SDL_KEYUP : SDL_KEYDOWN;
					keyEvent.key.state = (e.type == SDL_FINGERUP) ? SDL_RELEASED : SDL_PRESSED;
					
					// Determine direction based on touch position relative to bottom-left quadrant
					float jx = e.tfinger.x - 0.25f;
					float jy = e.tfinger.y - 0.75f;
					
					if (e.type != SDL_FINGERUP) {
						if (jy < -0.1f) keyEvent.key.keysym.scancode = SDL_SCANCODE_W;
						else if (jy > 0.1f) keyEvent.key.keysym.scancode = SDL_SCANCODE_S;
						else if (jx < -0.1f) keyEvent.key.keysym.scancode = SDL_SCANCODE_A;
						else if (jx > 0.1f) keyEvent.key.keysym.scancode = SDL_SCANCODE_D;
						else keyEvent.key.keysym.scancode = SDL_SCANCODE_SPACE; // Center tap to jump
					} else {
						// Release all movement keys on finger up
						keyEvent.key.keysym.scancode = SDL_SCANCODE_W; Keyboard::detail::pushEvent(keyEvent);
						keyEvent.key.keysym.scancode = SDL_SCANCODE_A; Keyboard::detail::pushEvent(keyEvent);
						keyEvent.key.keysym.scancode = SDL_SCANCODE_S; Keyboard::detail::pushEvent(keyEvent);
						keyEvent.key.keysym.scancode = SDL_SCANCODE_D; Keyboard::detail::pushEvent(keyEvent);
						keyEvent.key.keysym.scancode = SDL_SCANCODE_SPACE; Keyboard::detail::pushEvent(keyEvent);
						continue;
					}
					Keyboard::detail::pushEvent(keyEvent);
				} else {
					// Right half: Mouse look & tap to click
					if (e.type == SDL_FINGERMOTION) {
						SDL_Event mouseEvent;
						mouseEvent.type = SDL_MOUSEMOTION;
						mouseEvent.motion.xrel = e.tfinger.dx * 1000.0f; // Scale sensitivity
						mouseEvent.motion.yrel = e.tfinger.dy * 1000.0f;
						Mouse::detail::pushEvent(mouseEvent);
					} else {
						SDL_Event mouseEvent;
						mouseEvent.type = (e.type == SDL_FINGERDOWN) ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
						mouseEvent.button.button = SDL_BUTTON_LEFT;
						mouseEvent.button.state = (e.type == SDL_FINGERDOWN) ? SDL_PRESSED : SDL_RELEASED;
						Mouse::detail::pushEvent(mouseEvent);
					}
				}
				break;
			}
#endif
		}
	}

	
	if (!current_display_mode.isFullscreen())
	{
		int w, h;
		SDL_GetWindowSize(GLContext::detail::getWindow(), &w, &h);
		current_display_mode = DisplayMode(w, h);
	}
}

void swapBuffers()
{
	SDL_GL_SwapWindow(GLContext::detail::getWindow());
#if defined(B173_GL_TRACE)
	GLTrace::nextFrame();
#endif
}

void update(bool doProcessMessages)
{
	swapBuffers();
	if (doProcessMessages)
		processMessages();
}

void create(bool hidden)
{
	if (hidden)
		SDL_HideWindow(GLContext::detail::getWindow());
	else
		SDL_ShowWindow(GLContext::detail::getWindow());
}

int_t getX()
{
	int x;
	SDL_GetWindowPosition(GLContext::detail::getWindow(), &x, nullptr);
	return x;
}

int_t getY()
{
	int y;
	SDL_GetWindowPosition(GLContext::detail::getWindow(), nullptr, &y);
	return y;
}

int_t getWidth()
{
	return current_display_mode.getWidth();
}

int_t getHeight()
{
	return current_display_mode.getHeight();
}

}
}
