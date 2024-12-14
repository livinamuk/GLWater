#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Types.h"

#define NEAR_PLANE 0.1f
#define FAR_PLANE 5000.0f

namespace Camera {
    void Init(GLFWwindow* window);
    void Update(float deltaTime);
    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();
    glm::vec3 GetViewPos();
    Transform GetTransform();
}