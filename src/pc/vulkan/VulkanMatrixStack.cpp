#include "VulkanMatrixStack.h"
#include "GLState.h"
#include <iostream>

VulkanMatrixStack& VulkanMatrixStack::get() {
    static VulkanMatrixStack instance;
    return instance;
}

VulkanMatrixStack::VulkanMatrixStack() {
    modelViewStack.push_back(glm::mat4(1.0f));
    projectionStack.push_back(glm::mat4(1.0f));
}

void VulkanMatrixStack::matrixMode(VulkanMatrixMode mode) {
    currentMode = mode;
}

void VulkanMatrixStack::pushMatrix() {
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.push_back(modelViewStack.back());
    } else {
        projectionStack.push_back(projectionStack.back());
    }
}

void VulkanMatrixStack::popMatrix() {
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        if (modelViewStack.size() > 1) {
            modelViewStack.pop_back();
        } else {
            std::cerr << "ModelView Matrix stack underflow!" << std::endl;
        }
    } else {
        if (projectionStack.size() > 1) {
            projectionStack.pop_back();
        } else {
            std::cerr << "Projection Matrix stack underflow!" << std::endl;
        }
    }
}

void VulkanMatrixStack::loadIdentity() {
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() = glm::mat4(1.0f);
    } else {
        projectionStack.back() = glm::mat4(1.0f);
    }
}

void VulkanMatrixStack::loadMatrixf(const float* m) {
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() = glm::make_mat4(m);
    } else {
        projectionStack.back() = glm::make_mat4(m);
    }
}

void VulkanMatrixStack::multMatrixf(const float* m) {
    glm::mat4 matrix = glm::make_mat4(m);
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() *= matrix;
    } else {
        projectionStack.back() *= matrix;
    }
}

void VulkanMatrixStack::translatef(float x, float y, float z) {
    // Font bakes the glyph advance into its display lists
    if (GLState::isRecording()) {
        GLState::recordTranslate(x, y, z);
        return;
    }
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() = glm::translate(modelViewStack.back(), glm::vec3(x, y, z));
    } else {
        projectionStack.back() = glm::translate(projectionStack.back(), glm::vec3(x, y, z));
    }
}

void VulkanMatrixStack::rotatef(float angle, float x, float y, float z) {
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() = glm::rotate(modelViewStack.back(), glm::radians(angle), glm::vec3(x, y, z));
    } else {
        projectionStack.back() = glm::rotate(projectionStack.back(), glm::radians(angle), glm::vec3(x, y, z));
    }
}

void VulkanMatrixStack::scalef(float x, float y, float z) {
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() = glm::scale(modelViewStack.back(), glm::vec3(x, y, z));
    } else {
        projectionStack.back() = glm::scale(projectionStack.back(), glm::vec3(x, y, z));
    }
}

void VulkanMatrixStack::ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
    glm::mat4 orthoMat = glm::ortho(left, right, bottom, top, zNear, zFar);
    // Vulkan clip space has inverted Y and half Z. GLM ortho can be adjusted by Vulkan clip if using GLM_FORCE_DEPTH_ZERO_TO_ONE, but we'll manually apply a flip later if needed.
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() *= orthoMat;
    } else {
        projectionStack.back() *= orthoMat;
    }
}

void VulkanMatrixStack::frustum(float left, float right, float bottom, float top, float zNear, float zFar) {
    glm::mat4 frustumMat = glm::frustum(left, right, bottom, top, zNear, zFar);
    if (currentMode == VulkanMatrixMode::MODELVIEW) {
        modelViewStack.back() *= frustumMat;
    } else {
        projectionStack.back() *= frustumMat;
    }
}

glm::mat4 VulkanMatrixStack::getMVP() const {
    // Note: Vulkan needs projection to map Y downwards usually, but we can do that in shader or here.
    // We will just do P * V
    glm::mat4 clip = glm::mat4(1.0f);
    clip[1][1] = -1.0f; // Flip Y for Vulkan
    clip[2][2] = 0.5f; // Z from [-1, 1] to [0, 1]
    clip[3][2] = 0.5f;
    return clip * projectionStack.back() * modelViewStack.back();
}

glm::mat4 VulkanMatrixStack::getModelView() const {
    return modelViewStack.back();
}

glm::mat4 VulkanMatrixStack::getProjection() const {
    return projectionStack.back();
}
