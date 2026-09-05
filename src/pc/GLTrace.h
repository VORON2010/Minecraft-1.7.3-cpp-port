#pragma once

#if defined(B173_GL_TRACE)

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <glad/glad.h>

namespace GLTrace
{

bool enabled();
void nextFrame();
void begin(const char *operation);
void end();
void argument(float value);
void argument(double value);
void signedArgument(std::int64_t value);
void unsignedArgument(std::uint64_t value);
void argument(const char *value);

template<typename T>
typename std::enable_if<std::is_integral<T>::value>::type argument(T value)
{
	if (std::is_signed<T>::value)
		signedArgument(static_cast<std::int64_t>(value));
	else
		unsignedArgument(static_cast<std::uint64_t>(value));
}

struct Values
{
	const void *data;
	GLenum type;
	std::size_t count;
};

struct Pixels
{
	const void *data;
	GLsizei width;
	GLsizei height;
	GLenum format;
	GLenum type;
};

struct ArrayPointer
{
	GLenum array;
	GLint size;
	GLenum type;
	GLsizei stride;
	const void *data;
};

struct DrawArrays
{
	GLint first;
	GLsizei count;
};

void argument(const Values &values);
void argument(const Pixels &pixels);
void argument(const ArrayPointer &pointer);
void argument(const DrawArrays &draw);
void clientState(GLenum array, bool enabled);
void bindBuffer(GLenum target, GLuint buffer);
void pixelStore(GLenum pname, GLint param);
std::size_t lightCount(GLenum pname);

inline void arguments()
{
}

template<typename First, typename... Rest>
void arguments(const First &first, const Rest &...rest)
{
	argument(first);
	arguments(rest...);
}

template<typename... Args>
void record(const char *operation, const Args &...args)
{
	begin(operation);
	arguments(args...);
	end();
}
}

#else

namespace GLTrace
{
inline void nextFrame()
{
}
}

#endif
