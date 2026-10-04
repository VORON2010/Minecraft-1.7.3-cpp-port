#include "pc/vulkan/GLState.h"

#include <cmath>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <memory>

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include "client/renderer/Tesselator.h"
#include "pc/OpenGL.h"
#include "pc/vulkan/VulkanContext.h"
#include "pc/vulkan/VulkanMatrixStack.h"
#include "pc/vulkan/VulkanTexture.h"

namespace GLState
{
namespace
{
	// GL defaults, except depth func: Minecraft sets LEQUAL at init anyway
	float curColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float clearCol[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	bool texture2D = true;
	bool blend = false;
	bool depthTest = false;
	bool depthWrite = true;
	bool cull = false;
	bool alphaTest = false;
	bool fog = false;
	float alphaRef = 0.0f;
	uint32_t blendSrc = GL_ONE;
	uint32_t blendDst = GL_ZERO;
	uint32_t depthFn = GL_LEQUAL;
	uint32_t maskBits = 0;
	int32_t fogMode = GL_EXP;
	float fogStart = 0.0f, fogEnd = 1.0f, fogDensity = 1.0f;
	float fogCol[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	bool lighting = false;
	bool lightOn[2] = { false, false };
	glm::vec3 lightEyeDir[2] = { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
	float lightDiffuse[2] = { 0.0f, 0.0f };
	float lightAmbient = 0.2f;

	VkPipeline boundPipeline = VK_NULL_HANDLE;

	bool inBegin = false;
	uint32_t beginMode = 0;
	std::vector<TesselatorVertex> immediate;
	float curUV[2] = { 0.0f, 0.0f };
	int8_t curNormal[3] = { 0, 0, 127 };
	bool anyTextureBound = false;

	struct ListOp
	{
		enum Type { DRAW, TRANSLATE, COLOR } type;
		std::vector<TesselatorVertex> verts;
		int mode = 4;
		bool hasColor = false;
		bool hasTexture = false;
		float v[4] = {};
	};
	std::unordered_map<uint32_t, std::vector<ListOp>> lists;
	uint32_t recording = 0;
	bool isRecordingList = false;
	uint32_t base = 0;

	uint32_t blendIndex(uint32_t f)
	{
		if (f == GL_ZERO) return 0;
		if (f == GL_ONE) return 1;
		if (f >= 0x0300 && f <= 0x0307) return f - 0x0300 + 2;
		return 1;
	}

	uint32_t currentKey(bool lines)
	{
		uint32_t key = 0;
		if (blend)
		{
			key |= KEY_BLEND;
			key |= blendIndex(blendSrc) << KEY_SRC_SHIFT;
			key |= blendIndex(blendDst) << KEY_DST_SHIFT;
		}
		if (depthTest) key |= KEY_DEPTH_TEST;
		if (depthWrite) key |= KEY_DEPTH_WRITE;
		if (cull) key |= KEY_CULL;
		if (lines) key |= KEY_LINES;
		key |= maskBits << KEY_MASK_SHIFT;
		uint32_t fn = (depthFn >= 0x0200 && depthFn <= 0x0207) ? depthFn - 0x0200 : 3;
		key |= fn << KEY_DEPTH_FUNC_SHIFT;
		return key;
	}

	// Quads were already split by the tesselator, everything else becomes
	// a triangle or line list here.
	void expand(const TesselatorVertex *in, uint32_t n, int mode, std::vector<TesselatorVertex> &out, bool &lines)
	{
		lines = false;
		out.clear();
		switch (mode)
		{
		case GL_TRIANGLE_FAN:
			for (uint32_t i = 1; i + 1 < n; i++)
			{
				out.push_back(in[0]);
				out.push_back(in[i]);
				out.push_back(in[i + 1]);
			}
			break;
		case GL_TRIANGLE_STRIP:
			for (uint32_t i = 0; i + 2 < n; i++)
			{
				if (i & 1) { out.push_back(in[i + 1]); out.push_back(in[i]); }
				else { out.push_back(in[i]); out.push_back(in[i + 1]); }
				out.push_back(in[i + 2]);
			}
			break;
		case GL_LINE_STRIP:
			lines = true;
			for (uint32_t i = 0; i + 1 < n; i++)
			{
				out.push_back(in[i]);
				out.push_back(in[i + 1]);
			}
			break;
		case GL_LINES:
			lines = true;
			out.assign(in, in + (n & ~1u));
			break;
		default:
			out.assign(in, in + n);
			break;
		}
	}
}

void enable(uint32_t cap, bool on)
{
	switch (cap)
	{
	case GL_TEXTURE_2D: texture2D = on; break;
	case GL_BLEND: blend = on; break;
	case GL_DEPTH_TEST: depthTest = on; break;
	case GL_CULL_FACE: cull = on; break;
	case GL_ALPHA_TEST: alphaTest = on; break;
	case GL_FOG: fog = on; break;
	case GL_LIGHTING: lighting = on; break;
	case GL_LIGHT0: lightOn[0] = on; break;
	case GL_LIGHT1: lightOn[1] = on; break;
	default: break;
	}
}

void color(float r, float g, float b, float a)
{
	if (isRecordingList)
	{
		ListOp op;
		op.type = ListOp::COLOR;
		op.v[0] = r; op.v[1] = g; op.v[2] = b; op.v[3] = a;
		lists[recording].push_back(std::move(op));
		return;
	}
	// GL clamps glColor to [0, 1]
	auto c01 = [](float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };
	curColor[0] = c01(r); curColor[1] = c01(g); curColor[2] = c01(b); curColor[3] = c01(a);
}

void blendFunc(uint32_t src, uint32_t dst) { blendSrc = src; blendDst = dst; }
void depthMask(bool on) { depthWrite = on; }
void depthFunc(uint32_t func) { depthFn = func; }
void alphaFunc(uint32_t, float ref) { alphaRef = ref; }
void cullFace(uint32_t) {}

void lightfv(uint32_t light, uint32_t pname, const float *params)
{
	int i = light == GL_LIGHT0 ? 0 : (light == GL_LIGHT1 ? 1 : -1);
	if (i < 0) return;
	if (pname == GL_POSITION)
	{
		// GL stores light positions in eye space, using the modelview at call time
		glm::mat3 mv(VulkanMatrixStack::get().getModelView());
		lightEyeDir[i] = mv * glm::vec3(params[0], params[1], params[2]);
	}
	else if (pname == GL_DIFFUSE)
	{
		lightDiffuse[i] = params[0];
	}
}

void lightModelfv(uint32_t pname, const float *params)
{
	if (pname == GL_LIGHT_MODEL_AMBIENT) lightAmbient = params[0];
}

void colorMask(bool r, bool g, bool b, bool a)
{
	maskBits = (r ? 0 : 1) | (g ? 0 : 2) | (b ? 0 : 4) | (a ? 0 : 8);
}

void fogi(uint32_t pname, int32_t value)
{
	if (pname == GL_FOG_MODE) fogMode = value;
}

void fogf(uint32_t pname, float value)
{
	if (pname == GL_FOG_START) fogStart = value;
	else if (pname == GL_FOG_END) fogEnd = value;
	else if (pname == GL_FOG_DENSITY) fogDensity = value;
	else if (pname == GL_FOG_MODE) fogMode = static_cast<int32_t>(value);
}

void fogfv(uint32_t pname, const float *values)
{
	if (pname == GL_FOG_COLOR) std::memcpy(fogCol, values, sizeof(fogCol));
}

void clearColor(float r, float g, float b, float a)
{
	clearCol[0] = r; clearCol[1] = g; clearCol[2] = b; clearCol[3] = a;
}

const float *getClearColor() { return clearCol; }

void clear(uint32_t mask)
{
	VulkanContext &ctx = VulkanContext::getInstance();
	if (!ctx.getIsFrameActive()) return;

	VkClearAttachment att[2]{};
	uint32_t n = 0;
	if (mask & GL_COLOR_BUFFER_BIT)
	{
		att[n].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		att[n].colorAttachment = 0;
		std::memcpy(att[n].clearValue.color.float32, clearCol, sizeof(clearCol));
		n++;
	}
	if (mask & GL_DEPTH_BUFFER_BIT)
	{
		att[n].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		att[n].clearValue.depthStencil = { 1.0f, 0 };
		n++;
	}
	if (n == 0) return;

	VkClearRect rect{};
	rect.rect.extent = ctx.getSwapchainExtent();
	rect.layerCount = 1;
	vkCmdClearAttachments(ctx.getCurrentCommandBuffer(), n, att, 1, &rect);
}

void texImage(int32_t w, int32_t h, const void *pixels)
{
	if (pixels == nullptr || w <= 0 || h <= 0) return;
	VulkanContext &ctx = VulkanContext::getInstance();
	uint32_t id = g_boundTextureId;
	auto it = ctx.globalTextures.find(id);
	if (it != ctx.globalTextures.end() && it->second && it->second->getWidth() == w && it->second->getHeight() == h)
	{
		it->second->updateSub(0, 0, w, h, static_cast<const uint8_t *>(pixels));
		return;
	}

	auto tex = std::make_shared<VulkanTexture>();
	std::shared_ptr<std::vector<uint8_t>> copy = std::make_shared<std::vector<uint8_t>>(
		static_cast<const uint8_t *>(pixels), static_cast<const uint8_t *>(pixels) + static_cast<size_t>(w) * h * 4);
	ctx.pendingTextureUploads.push_back([tex, w, h, copy]() { tex->upload(w, h, copy->data()); });
	ctx.globalTextures[id] = tex;
}

void texSubImage(int32_t x, int32_t y, int32_t w, int32_t h, const void *pixels)
{
	VulkanContext &ctx = VulkanContext::getInstance();
	auto it = ctx.globalTextures.find(g_boundTextureId);
	if (it == ctx.globalTextures.end() || !it->second) return;
	it->second->updateSub(x, y, w, h, static_cast<const uint8_t *>(pixels));
}

void begin(uint32_t mode)
{
	inBegin = true;
	beginMode = mode;
	immediate.clear();
}

void texCoord(float u, float v)
{
	curUV[0] = u;
	curUV[1] = v;
}

void normal(float x, float y, float z)
{
	auto toByte = [](float f) -> int8_t {
		int v = static_cast<int>(f * 127.0f);
		return static_cast<int8_t>(v > 127 ? 127 : (v < -127 ? -127 : v));
	};
	curNormal[0] = toByte(x);
	curNormal[1] = toByte(y);
	curNormal[2] = toByte(z);
}

void vertex(float x, float y, float z)
{
	if (!inBegin) return;
	TesselatorVertex v{};
	v.pos[0] = x; v.pos[1] = y; v.pos[2] = z;
	v.uv[0] = curUV[0]; v.uv[1] = curUV[1];
	for (int i = 0; i < 4; i++)
		v.color[i] = static_cast<uint8_t>(curColor[i] * 255.0f + 0.5f);
	v.normal[0] = curNormal[0]; v.normal[1] = curNormal[1]; v.normal[2] = curNormal[2];
	immediate.push_back(v);
}

void end()
{
	if (!inBegin) return;
	inBegin = false;
	int mode = static_cast<int>(beginMode);
	if (mode == GL_QUADS)
	{
		// draw() expects quads already split into triangles
		std::vector<TesselatorVertex> tris;
		for (size_t i = 0; i + 3 < immediate.size(); i += 4)
		{
			const TesselatorVertex *q = &immediate[i];
			tris.insert(tris.end(), { q[0], q[1], q[2], q[0], q[2], q[3] });
		}
		immediate.swap(tris);
		mode = 4;
	}
	// Per-vertex colors carry glColor, the texture follows GL_TEXTURE_2D
	draw(immediate.data(), static_cast<uint32_t>(immediate.size()), mode, true, true);
	immediate.clear();
}

void newList(uint32_t id)
{
	lists[id].clear();
	recording = id;
	isRecordingList = true;
}

void endList() { isRecordingList = false; }
bool isRecording() { return isRecordingList; }

void recordTranslate(float x, float y, float z)
{
	ListOp op;
	op.type = ListOp::TRANSLATE;
	op.v[0] = x; op.v[1] = y; op.v[2] = z;
	lists[recording].push_back(std::move(op));
}

void callList(uint32_t id)
{
	if (isRecordingList) return;
	auto it = lists.find(id);
	if (it == lists.end()) return;
	for (const ListOp &op : it->second)
	{
		switch (op.type)
		{
		case ListOp::DRAW:
			draw(op.verts.data(), static_cast<uint32_t>(op.verts.size()), op.mode, op.hasColor, op.hasTexture);
			break;
		case ListOp::TRANSLATE:
			VulkanMatrixStack::get().translatef(op.v[0], op.v[1], op.v[2]);
			break;
		case ListOp::COLOR:
			color(op.v[0], op.v[1], op.v[2], op.v[3]);
			break;
		}
	}
}

void callLists(int32_t n, uint32_t type, const void *ids)
{
	for (int32_t i = 0; i < n; i++)
	{
		uint32_t id;
		if (type == GL_UNSIGNED_INT) id = static_cast<const uint32_t *>(ids)[i];
		else id = static_cast<const uint8_t *>(ids)[i];
		callList(base + id);
	}
}

void listBase(uint32_t b) { base = b; }

void deleteLists(uint32_t id, int32_t range)
{
	for (int32_t i = 0; i < range; i++)
		lists.erase(id + i);
}

bool bindState(VkCommandBuffer cmd, bool hasColor, bool lines)
{
	VulkanContext &ctx = VulkanContext::getInstance();

	VkPipeline pipeline = ctx.getPipeline(currentKey(lines));
	if (pipeline != boundPipeline)
	{
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		boundPipeline = pipeline;
	}

	auto &textures = ctx.globalTextures;
	auto it = textures.find(g_boundTextureId);
	bool haveTexture = it != textures.end() && it->second && it->second->bind(cmd, ctx.getPipelineLayout(), 0);
	if (haveTexture)
	{
		anyTextureBound = true;
	}
	else if (!anyTextureBound)
	{
		// The shader still declares the sampler, so something must be bound
		for (auto &t : textures)
		{
			if (t.second && t.second->bind(cmd, ctx.getPipelineLayout(), 0))
			{
				anyTextureBound = true;
				break;
			}
		}
		if (!anyTextureBound) return false;
	}

	PushConstants pc{};
	glm::mat4 mvp = VulkanMatrixStack::get().getMVP();
	std::memcpy(pc.mvp, glm::value_ptr(mvp), sizeof(pc.mvp));
	if (hasColor)
		pc.color[0] = pc.color[1] = pc.color[2] = pc.color[3] = 1.0f;
	else
		std::memcpy(pc.color, curColor, sizeof(curColor));
	std::memcpy(pc.fogColor, fogCol, sizeof(fogCol));
	pc.fogParams[0] = fogStart;
	pc.fogParams[1] = fogEnd;
	pc.fogParams[2] = fogDensity;
	pc.fogParams[3] = !fog ? 0.0f : (fogMode == GL_LINEAR ? 1.0f : 2.0f);
	pc.misc[0] = (texture2D && haveTexture) ? 1.0f : 0.0f;
	pc.misc[1] = alphaTest ? alphaRef : -1.0f;
	if (lighting)
	{
		// Normals stay in object space, so bring the lights there instead.
		// Exact for rotation + uniform scale, which is all the models use.
		glm::mat3 inv = glm::inverse(glm::mat3(VulkanMatrixStack::get().getModelView()));
		float *dst[2] = { pc.light0, pc.light1 };
		for (int i = 0; i < 2; i++)
		{
			glm::vec3 d = inv * lightEyeDir[i];
			float len = glm::length(d);
			if (len > 0.0f) d /= len;
			dst[i][0] = d.x; dst[i][1] = d.y; dst[i][2] = d.z;
			dst[i][3] = lightOn[i] ? lightDiffuse[i] : 0.0f;
		}
		pc.misc[2] = 1.0f;
		pc.misc[3] = lightAmbient;
	}
	vkCmdPushConstants(cmd, ctx.getPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);
	return true;
}

void draw(const TesselatorVertex *verts, uint32_t count, int mode, bool hasColor, bool hasTexture)
{
	if (count == 0) return;

	if (isRecordingList)
	{
		ListOp op;
		op.type = ListOp::DRAW;
		op.verts.assign(verts, verts + count);
		op.mode = mode;
		op.hasColor = hasColor;
		op.hasTexture = hasTexture;
		lists[recording].push_back(std::move(op));
		return;
	}

	VulkanContext &ctx = VulkanContext::getInstance();
	if (!ctx.getIsFrameActive()) return;

	static std::vector<TesselatorVertex> scratch;
	bool lines = false;
	const TesselatorVertex *data = verts;
	uint32_t n = count;
	if (mode != 4 && mode != GL_QUADS)
	{
		expand(verts, count, mode, scratch, lines);
		data = scratch.data();
		n = static_cast<uint32_t>(scratch.size());
		if (n == 0) return;
	}

	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceSize offset = 0;
	if (!ctx.streamVertices(data, n * sizeof(TesselatorVertex), buffer, offset)) return;

	VkCommandBuffer cmd = ctx.getCurrentCommandBuffer();
	vkCmdBindVertexBuffers(cmd, 0, 1, &buffer, &offset);
	if (bindState(cmd, hasColor, lines))
		vkCmdDraw(cmd, n, 1, 0, 0);
}

void onFrameBegin()
{
	boundPipeline = VK_NULL_HANDLE;
	anyTextureBound = false;
}
}
