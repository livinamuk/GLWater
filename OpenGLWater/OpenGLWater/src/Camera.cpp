#include "Camera.h"

#define NEAR_PLANE 0.01f
#define FAR_PLANE 5000.0f

namespace Camera {

    Transform gTransform;
    double gMouseX = 0;
    double gMouseY = 0;
    double gMouseOffsetX = 0;
    double gMouseOffsetY = 0;
    float gMouseSensitivity = 0.002f;
    float gWalkSpeed = 6.0f;
    GLFWwindow* gWindow;

    void Init(GLFWwindow* window) {
        gWindow = window;
        gTransform.position = glm::vec3(0, -0.15f, 0);
        gTransform.rotation = glm::vec3(-0.25f, 0.0f, 0.0f);
        double x, y;
        glfwGetCursorPos(gWindow, &x, &y);
        gMouseOffsetX = x;
        gMouseOffsetY = y;
        gMouseX = x;
        gMouseY = y;
    }

    void Update(float deltaTime) {
        // Mouselook
        double x, y;
        glfwGetCursorPos(gWindow, &x, &y);
        gMouseOffsetX = x - gMouseX;
        gMouseOffsetY = y - gMouseY;
        gMouseX = x;
        gMouseY = y;
        gTransform.rotation.x += -gMouseOffsetY * gMouseSensitivity;
        gTransform.rotation.y += -gMouseOffsetX * gMouseSensitivity;
        gTransform.rotation.x = std::min(gTransform.rotation.x, 1.5f);
        gTransform.rotation.x = std::max(gTransform.rotation.x, -1.5f);
        glm::vec3 camRight = glm::vec3(gTransform.to_mat4()[0]);
        glm::vec3 camForward = glm::vec3(gTransform.to_mat4()[2]);
        glm::vec3 movementForwardVector = glm::normalize(glm::vec3(camForward.x, 0, camForward.z));

        // WSAD
        glm::vec3 displacement = glm::vec3(0);
        if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS) {
            displacement -= camRight;
        }
        if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS) {
            displacement += camRight;
        }
        if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS) {
            displacement -= movementForwardVector;
        }
        if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS) {
            displacement += movementForwardVector;
        }
        displacement *= gWalkSpeed * deltaTime;
        gTransform.position += displacement;

        // Height
        float heightSpeed = 3.0f;
        if (glfwGetKey(gWindow, GLFW_KEY_Q) == GLFW_PRESS) {
            gTransform.position.y += deltaTime * heightSpeed;
        }
        if (glfwGetKey(gWindow, GLFW_KEY_E) == GLFW_PRESS) {
            gTransform.position.y -= deltaTime * heightSpeed;
        }
    }

    glm::mat4 GetProjectionMatrix() {
        int width, height;
        glfwGetWindowSize(gWindow, &width, &height);
        return glm::perspective(1.0f, float(width) / float(height), NEAR_PLANE, FAR_PLANE);
    }

    glm::mat4 GetViewMatrix() {
        return glm::inverse(gTransform.to_mat4());
    }

    glm::vec3 GetViewPos() {
        return gTransform.position;
    }

    Transform GetTransform() {
        return gTransform;
    }
}