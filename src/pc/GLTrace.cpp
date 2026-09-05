#include "GLTrace.h"

#if defined(B173_GL_TRACE)

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>

#if defined(_MSC_VER)
#include <share.h>
#endif

namespace GLTrace
{
struct ClientArray
{
	bool enabled = false;
	GLint size = 0;
	GLenum type = 0;
	GLsizei stride = 0;
	const void *data = nullptr;
	GLuint buffer = 0;
};

struct State
{
	std::FILE *file = nullptr;
	std::uint64_t frame = 0;
	std::uint64_t sequence = 0;
	GLuint arrayBuffer = 0;
	GLuint unpackBuffer = 0;
	GLint unpackAlignment = 4;
	GLint unpackRowLength = 0;
	GLint unpackSkipRows = 0;
	GLint unpackSkipPixels = 0;
	ClientArray arrays[4];

	State()
	{
#if defined(_MSC_VER)
		char *ownedPath = nullptr;
		std::size_t pathSize = 0;
		if (_dupenv_s(&ownedPath, &pathSize, "B173_GL_TRACE") != 0)
			return;
		const char *path = ownedPath;
#else
		const char *path = std::getenv("B173_GL_TRACE");
#endif
		if (path != nullptr && *path != '\0')
		{
#if defined(_MSC_VER)
			file = _fsopen(path, "wb", _SH_DENYNO);
#else
			file = std::fopen(path, "wb");
#endif
			if (file == nullptr)
				std::fprintf(stderr, "Cannot open B173_GL_TRACE output: %s\n", path);
		}
#if defined(_MSC_VER)
		std::free(ownedPath);
#endif
		if (file != nullptr)
			std::fputs("# b173-gl-trace-v1\tframe\tsequence\toperation\targuments\n", file);
	}

	~State()
	{
		if (file != nullptr)
			std::fclose(file);
	}
};

State &state()
{
	static State value;
	return value;
}

bool enabled()
{
	return state().file != nullptr;
}

void begin(const char *operation)
{
	State &s = state();
	std::fprintf(s.file, "%llu\t%llu\t%s", static_cast<unsigned long long>(s.frame),
		static_cast<unsigned long long>(s.sequence++), operation);
}

void end()
{
	State &s = state();
	std::fputc('\n', s.file);
	if (std::ferror(s.file))
	{
		std::fputs("B173_GL_TRACE write failed; tracing disabled.\n", stderr);
		std::fclose(s.file);
		s.file = nullptr;
	}
}

void nextFrame()
{
	if (!enabled())
		return;
	record("frame-end");
	State &s = state();
	++s.frame;
	if (s.file != nullptr)
		std::fflush(s.file);
}

void signedArgument(std::int64_t value)
{
	std::fprintf(state().file, "\ti:%lld", static_cast<long long>(value));
}

void unsignedArgument(std::uint64_t value)
{
	std::fprintf(state().file, "\tu:%llu", static_cast<unsigned long long>(value));
}

void argument(float value)
{
	std::uint32_t bits;
	static_assert(sizeof(bits) == sizeof(value), "GLfloat must be binary32");
	std::memcpy(&bits, &value, sizeof(bits));
	std::fprintf(state().file, "\tf32:%08x", static_cast<unsigned int>(bits));
}

void argument(double value)
{
	std::uint64_t bits;
	static_assert(sizeof(bits) == sizeof(value), "GLdouble must be binary64");
	std::memcpy(&bits, &value, sizeof(bits));
	std::fprintf(state().file, "\tf64:%016llx", static_cast<unsigned long long>(bits));
}

void argument(const char *value)
{
	std::fprintf(state().file, "\t%s", value);
}

std::size_t typeSize(GLenum type)
{
	switch (type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE: return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT: return 2;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT: return 4;
		case GL_DOUBLE: return 8;
		default: return 0;
	}
}

template<typename T>
void scalar(const unsigned char *data)
{
	T value;
	std::memcpy(&value, data, sizeof(value));
	argument(value);
}

void argument(const Values &values)
{
	if (values.data == nullptr)
	{
		argument("null");
		return;
	}
	const std::size_t size = typeSize(values.type);
	if (size == 0 || values.count > std::numeric_limits<std::size_t>::max() / size)
	{
		argument("unsupported-values");
		return;
	}
	argument("[");
	const unsigned char *data = static_cast<const unsigned char *>(values.data);
	for (std::size_t i = 0; i < values.count; ++i, data += size)
	{
		switch (values.type)
		{
			case GL_BYTE: scalar<GLbyte>(data); break;
			case GL_UNSIGNED_BYTE: scalar<GLubyte>(data); break;
			case GL_SHORT: scalar<GLshort>(data); break;
			case GL_UNSIGNED_SHORT: scalar<GLushort>(data); break;
			case GL_INT: scalar<GLint>(data); break;
			case GL_UNSIGNED_INT: scalar<GLuint>(data); break;
			case GL_FLOAT: scalar<GLfloat>(data); break;
			case GL_DOUBLE: scalar<GLdouble>(data); break;
		}
	}
	argument("]");
}

int arrayIndex(GLenum array)
{
	switch (array)
	{
		case GL_VERTEX_ARRAY: return 0;
		case GL_TEXTURE_COORD_ARRAY: return 1;
		case GL_COLOR_ARRAY: return 2;
		case GL_NORMAL_ARRAY: return 3;
		default: return -1;
	}
}

void clientState(GLenum array, bool value)
{
	const int index = arrayIndex(array);
	if (index >= 0)
		state().arrays[index].enabled = value;
}

void bindBuffer(GLenum target, GLuint buffer)
{
	if (target == GL_ARRAY_BUFFER)
		state().arrayBuffer = buffer;
	else if (target == GL_PIXEL_UNPACK_BUFFER)
		state().unpackBuffer = buffer;
}

void argument(const ArrayPointer &pointer)
{
	State &s = state();
	const int index = arrayIndex(pointer.array);
	if (index >= 0 && pointer.size >= 1 && pointer.size <= 4 && pointer.stride >= 0 && typeSize(pointer.type) != 0)
	{
		ClientArray &array = s.arrays[index];
		array.size = pointer.size;
		array.type = pointer.type;
		array.stride = pointer.stride;
		array.data = pointer.data;
		array.buffer = s.arrayBuffer;
	}
	if (s.arrayBuffer != 0)
	{
		argument("buffer-offset");
		argument(static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(pointer.data)));
	}
	else
		argument(pointer.data == nullptr ? "null" : "client-memory");
}

void argument(const DrawArrays &draw)
{
	static const char *names[] = {"vertices", "texcoords", "colors", "normals"};
	for (int index = 0; index < 4; ++index)
	{
		const ClientArray &array = state().arrays[index];
		if (!array.enabled)
			continue;
		argument(names[index]);
		if (array.buffer != 0)
		{
			argument("buffer-backed-see-glBufferData");
			argument(array.buffer);
			continue;
		}
		const std::size_t size = typeSize(array.type);
		if (array.data == nullptr || size == 0 || draw.first < 0 || draw.count < 0)
		{
			argument("unavailable-array");
			continue;
		}
		const std::size_t stride = array.stride == 0 ? size * array.size : static_cast<std::size_t>(array.stride);
		const std::size_t last = static_cast<std::size_t>(draw.first) + static_cast<std::size_t>(draw.count);
		if (stride == 0 || last > std::numeric_limits<std::size_t>::max() / stride)
		{
			argument("invalid-array-range");
			continue;
		}
		const unsigned char *data = static_cast<const unsigned char *>(array.data);
		for (std::size_t i = static_cast<std::size_t>(draw.first); i < last; ++i)
			argument(Values{data + i * stride, array.type, static_cast<std::size_t>(array.size)});
	}
}

void pixelStore(GLenum pname, GLint param)
{
	State &s = state();
	switch (pname)
	{
		case GL_UNPACK_ALIGNMENT:
			if (param == 1 || param == 2 || param == 4 || param == 8)
				s.unpackAlignment = param;
			break;
		case GL_UNPACK_ROW_LENGTH:
			if (param >= 0) s.unpackRowLength = param;
			break;
		case GL_UNPACK_SKIP_ROWS:
			if (param >= 0) s.unpackSkipRows = param;
			break;
		case GL_UNPACK_SKIP_PIXELS:
			if (param >= 0) s.unpackSkipPixels = param;
			break;
	}
}

void argument(const Pixels &pixels)
{
	const State &s = state();
	if (s.unpackBuffer != 0)
	{
		argument("pixel-buffer-offset");
		argument(static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(pixels.data)));
		return;
	}
	if (pixels.data == nullptr)
	{
		argument("null");
		return;
	}
	if (pixels.type != GL_UNSIGNED_BYTE || (pixels.format != GL_RGBA && pixels.format != GL_RGB)
		|| pixels.width < 0 || pixels.height < 0)
	{
		argument("unsupported-pixel-layout");
		return;
	}
	const std::uint64_t components = pixels.format == GL_RGBA ? 4 : 3;
	const std::uint64_t rowPixels = s.unpackRowLength == 0 ? pixels.width : s.unpackRowLength;
	const std::uint64_t stride = (rowPixels * components + s.unpackAlignment - 1) / s.unpackAlignment * s.unpackAlignment;
	const std::uint64_t offset = static_cast<std::uint64_t>(s.unpackSkipRows) * stride
		+ static_cast<std::uint64_t>(s.unpackSkipPixels) * components;
	const std::uint64_t rowBytes = static_cast<std::uint64_t>(pixels.width) * components;
	const std::uint64_t maxSize = std::numeric_limits<std::size_t>::max();
	if (offset > maxSize || rowBytes > maxSize - offset
		|| (pixels.height > 0 && stride > (maxSize - offset - rowBytes) / static_cast<std::uint64_t>(pixels.height)))
	{
		argument("invalid-pixel-range");
		return;
	}
	const unsigned char *data = static_cast<const unsigned char *>(pixels.data) + static_cast<std::size_t>(offset);
	for (GLsizei row = 0; row < pixels.height; ++row)
		argument(Values{data + static_cast<std::size_t>(row * stride), GL_UNSIGNED_BYTE, static_cast<std::size_t>(rowBytes)});
}

std::size_t lightCount(GLenum pname)
{
	switch (pname)
	{
		case GL_AMBIENT:
		case GL_DIFFUSE:
		case GL_SPECULAR:
		case GL_POSITION: return 4;
		case GL_SPOT_DIRECTION: return 3;
		default: return 1;
	}
}
}

#endif
