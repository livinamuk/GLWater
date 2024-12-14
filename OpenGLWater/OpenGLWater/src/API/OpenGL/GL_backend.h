#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

enum class WindowedMode { WINDOWED, FULLSCREEN };

namespace OpenGLBackend {

    void Init(int width, int height, std::string title);
    void ToggleFullscreen();
    void SetWindowedMode(const WindowedMode& windowedMode);
    void ToggleFullscreen();
    void ForceCloseWindow();
    double GetMouseX();
    double GetMouseY();
    int GetWindowWidth();
    int GetWindowHeight();
    void SwapBuffersPollEvents();
    bool WindowIsOpen();
    GLFWwindow* GetWindowPtr();

}