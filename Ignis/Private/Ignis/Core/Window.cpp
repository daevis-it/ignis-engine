#include "Ignis/Core/Window.h"
#include "Ignis/Core/Logger.h"
#include "Ignis/Events/ApplicationEvent.h"
#include "Ignis/Events/KeyEvent.h"
#include "Ignis/Events/MouseEvent.h"

#include <format>
#include <stdexcept>

namespace
{
    // glGetString può restituire nullptr se il contesto non è valido: passarlo
    // direttamente a std::format sarebbe un crash mentre si diagnostica un crash.
    const char* GLStringOrUnknown(unsigned int name)
    {
        const auto* value = glGetString(name);
        return value ? reinterpret_cast<const char*>(value) : "(sconosciuto)";
    }
}

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
        if (!m_Window)
        {
            // Prima qui c'era un return: l'oggetto restava vivo ma invalido, e
            // l'Application lo usava allegramente. Il dettaglio dell'errore GLFW
            // è già passato dall'error callback installata in GLFWContext.
            throw std::runtime_error(std::format(
                "Impossibile creare la finestra GLFW ({}x{}, \"{}\"). "
                "Il driver supporta OpenGL 3.3 Core?", width, height, title));
        }

        glfwMakeContextCurrent(m_Window);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            // ATTENZIONE: stiamo lanciando dal COSTRUTTORE, quindi ~Window NON verrà
            // chiamato e la finestra appena creata resterebbe appesa. La chiudiamo
            // a mano: è l'unico punto del codice in cui serve farlo.
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            throw std::runtime_error(
                "gladLoadGLLoader() non riuscita: i puntatori alle funzioni OpenGL non "
                "sono stati caricati. Senza questo controllo, la prima chiamata GL "
                "sarebbe stata un segfault senza spiegazione.");
        }

        // Quale driver ci ha risposto davvero. Non è decorazione: il giorno che
        // qualcosa renderizza diverso fra portatile e workstation, questa è la
        // prima riga che si guarda.
        IGNIS_CORE_INFO("OpenGL   Vendor: {}", GLStringOrUnknown(GL_VENDOR));
        IGNIS_CORE_INFO("       Renderer: {}", GLStringOrUnknown(GL_RENDERER));
        IGNIS_CORE_INFO("        Versione: {}", GLStringOrUnknown(GL_VERSION));

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