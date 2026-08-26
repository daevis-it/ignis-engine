#include "Ignis/Core/Window.h"

// glad PRIMA di GLFW: glad definisce i simboli OpenGL che GLFW dichiarerebbe per
// conto suo, e l'ordine invertito produce una valanga di errori di ridefinizione.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Ignis/Core/Base.h"
#include "Ignis/Core/Logger.h"
#include "Ignis/Renderer/GLDebug.h"
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
    Window::Window(const WindowProps& props)
    {
        m_Data.Title  = props.Title;
        m_Data.Width  = props.Width;
        m_Data.Height = props.Height;

        // OpenGL 4.5 Core: la versione minima con Direct State Access completo.
        // ATTENZIONE: questi hint chiedono un MINIMO, non una versione esatta. Un
        // driver può legittimamente restituire un contesto più recente (Mesa dà 4.6,
        // NVIDIA dà esattamente 4.5). Ciò che tiene allineate le macchine è GLAD,
        // generato per 4.5: i simboli più recenti non esistono nel progetto.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#if defined(IGNIS_DEBUG)
        // Contesto di debug: è il prerequisito di glDebugMessageCallback, e va
        // chiesto PRIMA di creare la finestra — dopo non si può più. Come per la
        // versione, è una RICHIESTA: il driver può ignorarla, e InitGLDebugOutput
        // verifica il risultato invece di dare per buona l'intenzione.
        // Solo in Debug: un contesto di debug con output sincrono costa prestazioni.
        //
        // GLFW_CONTEXT_DEBUG è il nome moderno (GLFW 3.4, glfw3.h riga 1067);
        // GLFW_OPENGL_DEBUG_CONTEXT ne è l'alias legacy, stesso valore.
        glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
#endif

        m_Window = glfwCreateWindow(static_cast<int>(props.Width),
                                    static_cast<int>(props.Height),
                                    m_Data.Title.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            // Il dettaglio dell'errore GLFW è già passato dall'error callback
            // installata in GLFWContext: qui diamo il contesto applicativo.
            throw std::runtime_error(std::format(
                "Impossibile creare la finestra GLFW ({}x{}, \"{}\"). "
                "Il driver supporta OpenGL 4.5 Core?", props.Width, props.Height, m_Data.Title));
        }

        glfwMakeContextCurrent(m_Window);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            // Stiamo lanciando dal COSTRUTTORE, quindi ~Window NON verrà chiamato e
            // la finestra appena creata resterebbe appesa. La chiudiamo a mano: è
            // l'unico punto del progetto in cui serve farlo.
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            throw std::runtime_error(
                "gladLoadGLLoader() non riuscita: i puntatori alle funzioni OpenGL non "
                "sono stati caricati. Senza questo controllo, la prima chiamata GL "
                "sarebbe stata un segfault senza spiegazione.");
        }

        // Subito dopo GLAD e prima di qualunque altra chiamata GL: da qui in avanti
        // ogni errore del driver ha una voce. Non è una gl* fuori posto — la
        // funzione vive in Private/Ignis/Renderer/, che e' il perimetro di D17.
        InitGLDebugOutput();

        IGNIS_CORE_INFO("GLAD ha caricato OpenGL {}.{}", GLVersion.major, GLVersion.minor);
        IGNIS_CORE_INFO("OpenGL   Vendor: {}", GLStringOrUnknown(GL_VENDOR));
        IGNIS_CORE_INFO("       Renderer: {}", GLStringOrUnknown(GL_RENDERER));
        IGNIS_CORE_INFO("        Versione: {}", GLStringOrUnknown(GL_VERSION));

        // glfwCreateWindow prende SCREEN COORDINATES, ma tutto ciò che riguarda il
        // disegno ragiona in PIXEL. Su HiDPI i due numeri differiscono, quindi le
        // dimensioni vere le chiediamo al framebuffer invece di fidarci dei props.
        int fbWidth = 0, fbHeight = 0;
        glfwGetFramebufferSize(m_Window, &fbWidth, &fbHeight);
        m_Data.Width  = static_cast<uint32_t>(fbWidth);
        m_Data.Height = static_cast<uint32_t>(fbHeight);

        SetVSync(props.VSync);

        glfwSetWindowUserPointer(m_Window, &m_Data);

        // ══════════════════════════════════════════════════════════════════════
        //  Callback GLFW → eventi Ignis
        //
        //  Sono funzioni C: non possono catturare stato, per questo ognuna
        //  recupera la propria WindowData dal puntatore utente.
        //
        //  Il controllo `if (data.EventCallback)` non è pigrizia: fra la creazione
        //  della finestra e SetEventCallback esiste una finestra temporale in cui
        //  la callback è vuota, e invocare una std::function vuota è UB.
        // ══════════════════════════════════════════════════════════════════════

        // FRAMEBUFFER, non WindowSize: questa dà pixel, ed è ciò che serve a glViewport.
        glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.Width  = static_cast<uint32_t>(width);
            data.Height = static_cast<uint32_t>(height);

            WindowResizeEvent event(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
            if (data.EventCallback) data.EventCallback(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            WindowCloseEvent event;
            if (data.EventCallback) data.EventCallback(event);
        });

        glfwSetWindowFocusCallback(m_Window, [](GLFWwindow* window, int focused)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            if (focused)
            {
                WindowFocusEvent event;
                if (data.EventCallback) data.EventCallback(event);
            }
            else
            {
                WindowLostFocusEvent event;
                if (data.EventCallback) data.EventCallback(event);
            }
        });

        glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xPos, int yPos)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            WindowMovedEvent event(xPos, yPos);
            if (data.EventCallback) data.EventCallback(event);
        });

        // --- Tastiera ---
        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    data.KeyRepeatCount[key] = 0;
                    KeyPressedEvent event(key, 0);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    // GLFW dice CHE si ripete, non quante volte: il conteggio è nostro.
                    const int count = ++data.KeyRepeatCount[key];
                    KeyPressedEvent event(key, count);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    data.KeyRepeatCount.erase(key);
                    KeyReleasedEvent event(key);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                default: break;
            }
        });

        // Carattere digitato, con layout e modificatori già applicati dal sistema.
        // NON è un duplicato di KeyPressed: serve all'input di testo (e a ImGui),
        // dove conta "quale carattere è uscito", non "quale tasto è sceso".
        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            KeyTypedEvent event(codepoint);
            if (data.EventCallback) data.EventCallback(event);
        });

        // --- Mouse ---
        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int /*mods*/)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                default: break;
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            MouseScrolledEvent event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            if (data.EventCallback) data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            MouseMovedEvent event(static_cast<float>(xPos), static_cast<float>(yPos));
            if (data.EventCallback) data.EventCallback(event);
        });

        IGNIS_CORE_INFO("Finestra \"{}\" creata: {}x{} pixel, VSync {}",
                        m_Data.Title, m_Data.Width, m_Data.Height,
                        m_Data.VSync ? "attivo" : "spento");
    }

    Window::~Window()
    {
        glfwDestroyWindow(m_Window);
    }

    void Window::SetVSync(bool enabled)
    {
        // Richiede un contesto corrente: va chiamata dopo glfwMakeContextCurrent.
        glfwSwapInterval(enabled ? 1 : 0);
        m_Data.VSync = enabled;
    }

    void Window::Update()
    {
        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}
