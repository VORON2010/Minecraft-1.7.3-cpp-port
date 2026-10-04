#include "pc/OpenGL.h"
#include "client/renderer/Tesselator.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cstring>

#include "java/Number.h"
#include "pc/vulkan/VulkanContext.h"
#include "pc/vulkan/VulkanTexture.h"
#include "pc/vulkan/GLState.h"
#include "pc/OpenGL.h"

Tesselator Tesselator::instance(MAX_FLOATS);

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDevice physicalDevice = VulkanContext::getInstance().getPhysicalDevice();
    if (!physicalDevice) return 0;
    
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
    
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

Tesselator::Tesselator(int_t size)
{
	this->size = size;
    vertices_data.reserve(size);
}

Tesselator::~Tesselator()
{
    cleanupBuffer();
}

void Tesselator::allocateBuffer() {
    VkDevice device = VulkanContext::getInstance().getDevice();
    if (!device) return;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size * sizeof(TesselatorVertex);
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        FILE* f = fopen("debug_log.txt", "w"); fprintf(f, "Tesselator failed\n"); fclose(f); abort();
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate TesselatorVertex buffer memory!");
    }

    vkBindBufferMemory(device, vertexBuffer, vertexMemory, 0);
}

void Tesselator::cleanupBuffer() {
    VkDevice device = VulkanContext::getInstance().getDevice();
    if (device) {
        if (vertexBuffer) {
            vkDestroyBuffer(device, vertexBuffer, nullptr);
            vertexBuffer = VK_NULL_HANDLE;
        }
        if (vertexMemory) {
            vkFreeMemory(device, vertexMemory, nullptr);
            vertexMemory = VK_NULL_HANDLE;
        }
    }
}

Tesselator Tesselator::getUniqueInstance(int_t size)
{
	return Tesselator(size);
}

void Tesselator::end()
{
	if (!tesselating)
		throw std::runtime_error("Not tesselating!");
	tesselating = false;

	if (vertices > 0 && capture != nullptr)
	{
		capture->data.insert(capture->data.end(), vertices_data.begin(), vertices_data.end());
		capture->vertices += vertices;
		capture->hasTexture = hasTexture;
		capture->hasColor = hasColor;
		capture->hasNormal = hasNormal;
		capture->mode = (draw_mode == 7 && TRIANGLE_MODE) ? 4 : draw_mode; // 4 is GL_TRIANGLES
	}
	else if (vertices > 0)
	{
		// Every immediate-mode batch gets its own slice of the per-frame
		// streaming buffer. The old single shared buffer was overwritten by
		// each end() while earlier draws recorded in the same command buffer
		// (and up to two frames still on the GPU) were reading it, so all
		// draws showed whatever batch was written last.
		int mode = (draw_mode == 7 && TRIANGLE_MODE) ? 4 : draw_mode;
		GLState::draw(vertices_data.data(), static_cast<uint32_t>(vertices_data.size()), mode, hasColor, hasTexture);
	}

	clear();
}

void Tesselator::render(VkCommandBuffer cmd) {
    VulkanContext& ctx = VulkanContext::getInstance();
    if (!ctx.getIsFrameActive() || vertices <= 0 || vertices_data.empty()) return;
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;
    if (!ctx.streamVertices(vertices_data.data(), vertices_data.size() * sizeof(TesselatorVertex), buffer, offset)) return;
    {
        VkBuffer vertexBuffers[] = {buffer};
        VkDeviceSize offsets[] = {offset};
        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        
        VulkanContext::getInstance().pushMVP();

        auto& globalTextures = VulkanContext::getInstance().globalTextures;
        auto it = globalTextures.find(g_boundTextureId);
        if (it != globalTextures.end() && it->second) {
            it->second->bind(cmd, VulkanContext::getInstance().getPipelineLayout(), 0);
        }

        vkCmdDraw(cmd, vertices, 1, 0, 0);
    }
}

void Tesselator::clear()
{
	vertices = 0;
    vertices_data.clear();
	count = 0;
}

void Tesselator::begin()
{
	begin(7); // GL_QUADS
}

void Tesselator::begin(int mode)
{
	if (tesselating)
		throw std::runtime_error("Already tesselating!");
	tesselating = true;

	clear();
	draw_mode = mode;
	hasNormal = false;
	hasColor = false;
	hasTexture = false;
	noColorFlag = false;
}

void Tesselator::tex(double u, double v)
{
	hasTexture = true;
	this->u = u;
	this->v = v;
}

void Tesselator::color(float r, float g, float b)
{
	color(Java::numberToInt(r * 255.0f), Java::numberToInt(g * 255.0f), Java::numberToInt(b * 255.0f));
}

void Tesselator::color(float r, float g, float b, float a)
{
	color(Java::numberToInt(r * 255.0f), Java::numberToInt(g * 255.0f), Java::numberToInt(b * 255.0f), Java::numberToInt(a * 255.0f));
}

void Tesselator::color(int_t r, int_t g, int_t b)
{
	color(r, g, b, 255);
}

void Tesselator::color(int_t r, int_t g, int_t b, int_t a)
{
	if (noColorFlag)
		return;

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (a > 255) a = 255;

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	if (a < 0) a = 0;

	hasColor = true;
	const unsigned char rgba[] = {static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b), static_cast<unsigned char>(a)};
	std::memcpy(&col, rgba, sizeof(col));
}

void Tesselator::vertexUV(double x, double y, double z, double u, double v)
{
	tex(u, v);
	vertex(x, y, z);
}

void Tesselator::vertex(double x, double y, double z)
{
	count++;

    TesselatorVertex vtx;
    vtx.pos[0] = static_cast<float>(x + xo);
    vtx.pos[1] = static_cast<float>(y + yo);
    vtx.pos[2] = static_cast<float>(z + zo);
    vtx.uv[0] = hasTexture ? static_cast<float>(u) : 0.0f;
    vtx.uv[1] = hasTexture ? static_cast<float>(v) : 0.0f;
    if (hasColor) {
        std::memcpy(vtx.color, &col, 4);
    } else {
        vtx.color[0] = 255; vtx.color[1] = 255; vtx.color[2] = 255; vtx.color[3] = 255;
    }
    vtx.normal[0] = hasNormal ? normalBytes[0] : 0;
    vtx.normal[1] = hasNormal ? normalBytes[1] : 0;
    vtx.normal[2] = hasNormal ? normalBytes[2] : 0;
    vtx.normal[3] = 0;

	if (draw_mode == 7 && TRIANGLE_MODE && (count % 4) == 0 && vertices_data.size() >= 3)
	{
		TesselatorVertex v0 = vertices_data[vertices_data.size() - 3];
		TesselatorVertex v2 = vertices_data[vertices_data.size() - 1];

		vertices_data.push_back(v0);
		vertices++;
		
		vertices_data.push_back(v2);
		vertices++;
	}

	vertices_data.push_back(vtx);
	vertices++;

	// Flush only on a quad boundary. With TRIANGLE_MODE a quad is 6 stored
	// vertices, so "vertices % 4" fired mid-quad: the quad was split, count
	// was reset, and the next 4th vertex read vertices_data[size() - 3] on a
	// nearly empty vector (out of bounds -> 0xC0000005, or torn triangles).
	if ((draw_mode != 7 || (count % 4) == 0) && vertices_data.size() >= static_cast<size_t>(size) - 32)
	{
		end();
		tesselating = true;
	}
}

void Tesselator::color(int_t rgb)
{
	int r = (rgb >> 16) & 0xFF;
	int g = (rgb >> 8) & 0xFF;
	int b = (rgb >> 0) & 0xFF;
	color(r, g, b);
}

void Tesselator::color(int_t rgb, int_t a)
{
	int r = (rgb >> 16) & 0xFF;
	int g = (rgb >> 8) & 0xFF;
	int b = (rgb >> 0) & 0xFF;
	color(r, g, b, a);
}

void Tesselator::noColor()
{
	noColorFlag = true;
}

void Tesselator::normal(float x, float y, float z)
{
	if (!tesselating)
		std::cout << "But..\n";

	hasNormal = true;
	// The packed value below keeps Java's x * 128 overflow; the shader gets a clean copy
	auto toByte = [](float f) -> int8_t {
		int v = Java::numberToInt(f * 127.0f);
		return static_cast<int8_t>(v > 127 ? 127 : (v < -127 ? -127 : v));
	};
	normalBytes[0] = toByte(x);
	normalBytes[1] = toByte(y);
	normalBytes[2] = toByte(z);
	uint_t bx = static_cast<uint_t>(Java::numberToInt(x * 128.0f)) & 255u;
	uint_t by = static_cast<uint_t>(Java::numberToInt(y * 127.0f)) & 255u;
	uint_t bz = static_cast<uint_t>(Java::numberToInt(z * 127.0f)) & 255u;
	
	if (bx & 128u) bx |= 0xffffff00u;
	if (by & 128u) by |= 0xffffff00u;
	if (bz & 128u) bz |= 0xffffff00u;
	normalValue = Java::intFromBits(bx | (by << 8) | (bz << 16));
}

void Tesselator::offset(double x, double y, double z)
{
	xo = x;
	yo = y;
	zo = z;
}

void Tesselator::addOffset(float x, float y, float z)
{
	xo += x;
	yo += y;
	zo += z;
}

void Tesselator::captureTo(MeshCapture *target)
{
	capture = target;
}
