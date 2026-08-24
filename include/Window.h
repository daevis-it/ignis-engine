#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    bool ShouldClose() const;
    void Update();
    GLFWwindow* GetNativeWindow() const { return m_Window; }
private:
    GLFWwindow* m_Window;
};