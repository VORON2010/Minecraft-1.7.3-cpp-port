#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>

struct TesselatorVertex;

// Fixed-function GL state emulated on top of the Vulkan renderer.
// OpenGL.h forwards the old gl* calls here; every draw goes through draw()
// or bindState(), which pick a pipeline variant and push color/fog/alpha.
namespace GLState
{
	// Pipeline key bits, see VulkanContext::getPipeline
	enum : uint32_t
	{
		KEY_BLEND = 1u << 0,
		KEY_SRC_SHIFT = 1,       // 4 bits
		KEY_DST_SHIFT = 5,       // 4 bits
		KEY_DEPTH_TEST = 1u << 9,
		KEY_DEPTH_WRITE = 1u << 10,
		KEY_CULL = 1u << 11,
		KEY_LINES = 1u << 12,
		KEY_MASK_SHIFT = 13,     // 4 bits, set = channel disabled
		KEY_DEPTH_FUNC_SHIFT = 17 // 3 bits, VkCompareOp
	};

	struct PushConstants
	{
		float mvp[16];
		float color[4];
		float fogColor[4];
		float fogParams[4]; // start, end, density, mode (0 off, 1 linear, 2 exp)
		float misc[4];      // useTexture, alphaRef (<0 = no alpha test), lighting, ambient
		float light0[4];    // object-space direction, diffuse
		float light1[4];
	};

	void enable(uint32_t cap, bool on);
	void color(float r, float g, float b, float a);
	void blendFunc(uint32_t src, uint32_t dst);
	void depthMask(bool on);
	void depthFunc(uint32_t func);
	void colorMask(bool r, bool g, bool b, bool a);
	void alphaFunc(uint32_t func, float ref);
	void cullFace(uint32_t mode);
	void lightfv(uint32_t light, uint32_t pname, const float *params);
	void lightModelfv(uint32_t pname, const float *params);
	void fogi(uint32_t pname, int32_t value);
	void fogf(uint32_t pname, float value);
	void fogfv(uint32_t pname, const float *values);
	void clearColor(float r, float g, float b, float a);
	const float *getClearColor();
	void clear(uint32_t mask);

	// Both act on the texture bound with glBindTexture, RGBA8 only
	void texImage(int32_t w, int32_t h, const void *pixels);
	void texSubImage(int32_t x, int32_t y, int32_t w, int32_t h, const void *pixels);

	// glBegin/glEnd immediate mode, vertices take the current color/uv/normal
	void begin(uint32_t mode);
	void vertex(float x, float y, float z);
	void texCoord(float u, float v);
	void normal(float x, float y, float z);
	void end();

	void newList(uint32_t id);
	void endList();
	void callList(uint32_t id);
	void callLists(int32_t n, uint32_t type, const void *lists);
	void listBase(uint32_t base);
	void deleteLists(uint32_t id, int32_t range);
	bool isRecording();
	void recordTranslate(float x, float y, float z);

	// Immediate draw of tesselator output in any GL primitive mode
	void draw(const TesselatorVertex *verts, uint32_t count, int mode, bool hasColor, bool hasTexture);
	// Binds pipeline, texture and push constants for a draw the caller issues itself.
	// False when no texture is uploaded yet, the draw must be skipped then.
	bool bindState(VkCommandBuffer cmd, bool hasColor, bool lines);
	void onFrameBegin();
}
