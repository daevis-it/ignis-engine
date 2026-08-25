#pragma once

namespace Ignis
{
    // ══════════════════════════════════════════════════════════════════════════
    //  RAII sul contesto globale di GLFW.
    //
    //  Esiste per una ragione sola: rendere l'ordine init/terminate una
    //  conseguenza delle regole del linguaggio invece che dell'attenzione di chi
    //  scrive. Application lo dichiara PRIMA di m_Window, quindi — i membri si
    //  distruggono in ordine inverso — GLFW viene terminata DOPO che la finestra
    //  è stata distrutta. Che è l'unico ordine corretto.
    //
    //  Vive in Private/ apposta: il fatto che sotto ci sia GLFW è un dettaglio
    //  interno, e Application.h lo conosce solo come tipo dichiarato in avanti.
    // ══════════════════════════════════════════════════════════════════════════
    class GLFWContext
    {
    public:
        // Installa l'error callback, poi inizializza GLFW.
        // LANCIA std::runtime_error se GLFW non parte.
        GLFWContext();
        ~GLFWContext();

        // Regola ferrea: chi possiede una risorsa dichiara come si copia e come si
        // sposta. Questo possiede uno stato GLOBALE di libreria — copiarlo o
        // spostarlo non significa niente, e produrrebbe una doppia glfwTerminate().
        GLFWContext(const GLFWContext&)            = delete;
        GLFWContext& operator=(const GLFWContext&) = delete;
        GLFWContext(GLFWContext&&)                 = delete;
        GLFWContext& operator=(GLFWContext&&)      = delete;
    };
}
