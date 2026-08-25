#include "Ignis/Core/GLFWContext.h"
#include "Ignis/Core/Logger.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

namespace Ignis
{
    namespace
    {
        void GLFWErrorCallback(int code, const char* description)
        {
            IGNIS_CORE_ERROR("GLFW [{}]: {}", code,
                             description ? description : "(nessuna descrizione)");
        }
    }

    GLFWContext::GLFWContext()
    {
        // PRIMA di glfwInit, non dopo: GLFW lo consente apposta, ed è l'unico modo
        // di vedere anche gli errori dell'inizializzazione stessa. Finché questa
        // riga non c'era, glfwInit poteva fallire senza dire perché.
        glfwSetErrorCallback(GLFWErrorCallback);

        if (!glfwInit())
            throw std::runtime_error(
                "glfwInit() non riuscita. Su Linux di solito significa nessun display "
                "disponibile (sessione senza X11/Wayland, oppure DISPLAY non impostata).");

        int major = 0, minor = 0, revision = 0;
        glfwGetVersion(&major, &minor, &revision);
        IGNIS_CORE_INFO("GLFW inizializzata (versione {}.{}.{})", major, minor, revision);
    }

    GLFWContext::~GLFWContext()
    {
        // Se sei qui, la finestra è già stata distrutta: lo garantisce l'ordine
        // di dichiarazione dei membri in Application.
        glfwTerminate();
        IGNIS_CORE_INFO("GLFW terminata.");
    }
}
