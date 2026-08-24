#include "Window.h"
#include "Logger.h"

Window::Window(int width, int height, const std::string& title) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        Logger::Error("Impossibile creare la finestra GLFW!");
        return;
    }

    glfwMakeContextCurrent(m_Window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::Error("Impossibile inizializzare GLAD!");
        return;
    }

    // Esempio di utilizzo C++20 con parametri
    Logger::Info("Finestra creata con successo: {}x{}", width, height);
}

Window::~Window() {
    glfwDestroyWindow(m_Window);
}

bool Window::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

void Window::Update() {
    glfwSwapBuffers(m_Window);
    glfwPollEvents();
}
