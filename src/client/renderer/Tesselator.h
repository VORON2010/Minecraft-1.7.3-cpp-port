#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

#include "java/Type.h"

// Removed OpenGL.h
// #include "OpenGL.h"

#include "util/Memory.h"

struct TesselatorVertex {
    float pos[3];
    float uv[2];
    uint8_t color[4];
    int8_t normal[4];
};

struct MeshCapture
{
	std::vector<TesselatorVertex> data;
	int_t vertices = 0;
	bool hasTexture = false;
	bool hasColor = false;
	bool hasNormal = false;
	int mode = 0; 
};

class Tesselator
{
private:
	static constexpr bool TRIANGLE_MODE = true;
	static constexpr int_t MAX_MEMORY_USE = 0x1000000;
	static constexpr int_t MAX_FLOATS = 0x200000;

	std::vector<TesselatorVertex> vertices_data;

	int_t vertices = 0;

	double u = 0.0, v = 0.0;
	int_t col = 0;

	bool hasColor = false;
	bool hasTexture = false;
	bool hasNormal = false;

	int_t count = 0;
	bool noColorFlag = false;

	int draw_mode = 0; // previously GLenum

	double xo = 0.0;
	double yo = 0.0;
	double zo = 0.0;

	int_t normalValue = 0;
	int8_t normalBytes[3] = { 0, 0, 0 };

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vertexMemory = VK_NULL_HANDLE;

public:
	static Tesselator instance;

private:
	bool tesselating = false;

	int_t size = 0;
	MeshCapture *capture = nullptr;
	
	void allocateBuffer();
	void cleanupBuffer();

public:
	Tesselator(int_t size);
	~Tesselator();

	Tesselator getUniqueInstance(int_t size);

	void end();
	void render(VkCommandBuffer cmd);
	void clear();
	void begin();
	void begin(int mode);
	void tex(double u, double v);
	void color(float r, float g, float b);
	void color(float r, float g, float b, float a);
	void color(int_t r, int_t g, int_t b);
	void color(int_t r, int_t g, int_t b, int_t a);
	void vertexUV(double x, double y, double z, double u, double v);
	void vertex(double x, double y, double z);
	void color(int_t rgb);
	void color(int_t rgb, int_t a);
	void noColor();
	void normal(float x, float y, float z);
	void offset(double x, double y, double z);
	void addOffset(float x, float y, float z);
	
	void captureTo(MeshCapture *target);
};
