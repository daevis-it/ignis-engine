#include "Ignis/Core/Window.h"
#include "Ignis/Core/Logger.h"
#include "Ignis/Events/ApplicationEvent.h"
#include "Ignis/Events/KeyEvent.h"
#include "Ignis/Events/MouseEvent.h"

namespace Ignis
{
    Window::Window(int width, int height, const std::string& title) {
        m_Data.Width = width;
        m_Data.Height = height;
        m_Data.Title = title;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!m_Window) {
            IGNIS_CORE_ERROR("Impossibile creare la finestra GLFW!");
            return;
        }

        glfwMakeContextCurrent(m_Window);
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

        // Diciamo a GLFW di associare la nostra struttura dati m_Data a questa finestra
        glfwSetWindowUserPointer(m_Window, &m_Data);

        // --- CALLBACKS DI GLFW ---

        // Evento Ridimensionamento
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            // Recuperiamo i nostri dati
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            // Creiamo l'evento e lo spediamo all'engine!
            WindowResizeEvent event(width, height);
            data.EventCallback(event);
        });

        // Evento Chiusura
        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.EventCallback(event);
        });

        // --- CALLBACK DELLA TASTIERA ---
        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    // Se teniamo premuto, il repeat count è semplicemente > 0
                    KeyPressedEvent event(key, 1);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        // --- CALLBACK DEL MOUSE ---
        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event(button);
                    data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseMovedEvent event((float)xPos, (float)yPos);
            data.EventCallback(event);
        });

        IGNIS_CORE_INFO("Finestra creata con successo: {}x{}", width, height);
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
    }

    void Window::Update() {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}