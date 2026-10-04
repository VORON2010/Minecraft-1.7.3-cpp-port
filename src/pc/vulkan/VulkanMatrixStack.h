#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

enum class VulkanMatrixMode {
    MODELVIEW,
    PROJECTION
};

class VulkanMatrixStack {
public:
    static VulkanMatrixStack& get();

    void matrixMode(VulkanMatrixMode mode);
    void pushMatrix();
    void popMatrix();
    void loadIdentity();
    void loadMatrixf(const float* m);
    void multMatrixf(const float* m);
    void translatef(float x, float y, float z);
    void rotatef(float angle, float x, float y, float z);
    void scalef(float x, float y, float z);
    void ortho(float left, float right, float bottom, float top, float zNear, float zFar);
    void frustum(float left, float right, float bottom, float top, float zNear, float zFar);

    glm::mat4 getMVP() const;
    glm::mat4 getModelView() const;
    glm::mat4 getProjection() const;

private:
    VulkanMatrixStack();

    VulkanMatrixMode currentMode = VulkanMatrixMode::MODELVIEW;
    
    std::vector<glm::mat4> modelViewStack;
    std::vector<glm::mat4> projectionStack;
};
