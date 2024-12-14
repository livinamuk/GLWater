#pragma once
#include "GL_backend.h"
#include <string>
#include <iostream>

namespace OpenGLBackend {

    GLFWwindow* g_window;
    WindowedMode g_windowedMode;
    GLFWmonitor* g_monitor;
    const GLFWvidmode* g_mode;
    GLuint g_currentWindowWidth = 0;
    GLuint g_currentWindowHeight = 0;
    GLuint g_windowedWidth = 0;
    GLuint g_windowedHeight = 0;
    GLuint g_fullscreenWidth;
    GLuint g_fullscreenHeight;

    GLFWwindow* GetWindowPtr() {
        return g_window;
    }

    void Init(int width, int height, std::string title) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        g_monitor = glfwGetPrimaryMonitor();
        g_mode = glfwGetVideoMode(g_monitor);
        glfwWindowHint(GLFW_RED_BITS, g_mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, g_mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, g_mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, g_mode->refreshRate);
        g_fullscreenWidth = g_mode->width;
        g_fullscreenHeight = g_mode->height;
        g_windowedWidth = width;
        g_windowedHeight = height;
        g_window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
        if (g_window == NULL) {
            std::cout << "GLFW window failed creation\n";
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(g_window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "GLAD failed init\n";
            return;
        }
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        ToggleFullscreen();
        ToggleFullscreen();

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        SwapBuffersPollEvents();

        return;
    }

    void SetWindowedMode(const WindowedMode& windowedMode) {
        if (windowedMode == WindowedMode::WINDOWED) {
            g_currentWindowWidth = g_windowedWidth;
            g_currentWindowHeight = g_windowedHeight;
            glfwSetWindowMonitor(g_window, nullptr, 0, 0, g_windowedWidth, g_windowedHeight, g_mode->refreshRate);
            glfwSetWindowPos(g_window, 0, 0);
            glfwSwapInterval(0);
        }
        else if (windowedMode == WindowedMode::FULLSCREEN) {
            g_currentWindowWidth = g_fullscreenWidth;
            g_currentWindowHeight = g_fullscreenHeight;
            glfwSetWindowMonitor(g_window, g_monitor, 0, 0, g_fullscreenWidth, g_fullscreenHeight, g_mode->refreshRate); 
            glfwSwapInterval(1);
        }
        g_windowedMode = windowedMode;
    }

    void ToggleFullscreen() {
        if (g_windowedMode == WindowedMode::WINDOWED) {
            SetWindowedMode(WindowedMode::FULLSCREEN);
        }
        else {
            SetWindowedMode(WindowedMode::WINDOWED);
        }
    }

    void ForceCloseWindow() {
        glfwSetWindowShouldClose(g_window, true);
    }

    double GetMouseX() {
        double x, y;
        glfwGetCursorPos(g_window, &x, &y);
        return x;
    }

    double GetMouseY() {
        double x, y;
        glfwGetCursorPos(g_window, &x, &y);
        return y;
    }

    int GetWindowWidth() {
        int width, height;
        glfwGetWindowSize(g_window, &width, &height);
        return width;
    }

    int GetWindowHeight() {
        int width, height;
        glfwGetWindowSize(g_window, &width, &height);
        return height;
    }

    void SwapBuffersPollEvents() {
        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    bool WindowIsOpen() {
        return !glfwWindowShouldClose(g_window);
    }
}