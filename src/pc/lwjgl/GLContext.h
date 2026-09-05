#pragma once

#include <string>
#include <set>

#include "SDL.h"
#include "glad/glad.h"

namespace lwjgl
{
namespace GLContext
{


namespace detail
{


struct GLCapabilities
{
private:
	std::set<std::string> caps;

public:
	void add(const std::string &cap)
	{
		caps.insert(cap);
	}

	bool operator[](const std::string &cap) const
	{
		return caps.find(cap) != caps.end();
	}
};


SDL_Window *getWindow();
SDL_GLContext getGLContext();

}


void instantiate();
const detail::GLCapabilities &getCapabilities();

}
}
