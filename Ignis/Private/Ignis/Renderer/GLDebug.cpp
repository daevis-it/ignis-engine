#include "Ignis/Renderer/GLDebug.h"

#include "Ignis/Core/Base.h"
#include "Ignis/Core/Logger.h"

#include <glad/glad.h>

namespace Ignis
{
    namespace
    {
        const char* SorgenteToString(GLenum source)
        {
            switch (source)
            {
                case GL_DEBUG_SOURCE_API:             return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WindowSystem";
                case GL_DEBUG_SOURCE_SHADER_COMPILER: return "ShaderCompiler";
                case GL_DEBUG_SOURCE_THIRD_PARTY:     return "ThirdParty";
                case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
                case GL_DEBUG_SOURCE_OTHER:           return "Other";
                default:                              return "?";
            }
        }

        const char* TipoToString(GLenum type)
        {
            switch (type)
            {
                case GL_DEBUG_TYPE_ERROR:               return "ERRORE";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATO";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UB";
                case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITA";
                case GL_DEBUG_TYPE_PERFORMANCE:         return "PRESTAZIONI";
                case GL_DEBUG_TYPE_MARKER:              return "MARKER";
                case GL_DEBUG_TYPE_PUSH_GROUP:          return "PUSH";
                case GL_DEBUG_TYPE_POP_GROUP:           return "POP";
                case GL_DEBUG_TYPE_OTHER:               return "ALTRO";
                default:                                return "?";
            }
        }

        // APIENTRY è OBBLIGATORIO e non è decorazione: su Windows vale __stdcall,
        // su Linux è vuoto (glad.h, righe 654-658). Senza, MSVC rifiuta la
        // conversione a GLDEBUGPROC — e se per qualche motivo la accettasse, lo
        // stack verrebbe pulito dalla parte sbagliata.
        //
        // Il messaggio del driver è una stringa RUNTIME, quindi non può essere il
        // format string del Logger (è std::format_string, verificato dal
        // compilatore): passa come ARGOMENTO. È la stessa lezione del vecchio
        // Logger::Info(e.ToString()).
        void APIENTRY GLDebugCallback(GLenum source, GLenum type, GLuint id,
                                      GLenum severity, GLsizei /*length*/,
                                      const GLchar* message, const void* /*userParam*/)
        {
            const char* sorgente = SorgenteToString(source);
            const char* tipo     = TipoToString(type);
            const char* testo    = message ? message : "(nessun messaggio)";

            switch (severity)
            {
                case GL_DEBUG_SEVERITY_HIGH:
                    IGNIS_CORE_ERROR("GL {} [{}] #{}: {}", tipo, sorgente, id, testo);
                    // Con l'output SINCRONO, aggiungere qui IGNIS_DEBUGBREAK() ferma il
                    // processo sulla riga colpevole — comodissimo. NON è il default di
                    // proposito: i messaggi ad alta severità arrivano anche da codice di
                    // terzi (i backend ImGui, il driver), e un trap all'avvio su una sola
                    // delle due macchine sarebbe un falso positivo costoso. Questo task
                    // fa parlare la GPU; farle uccidere il processo è un'altra decisione.
                    break;

                case GL_DEBUG_SEVERITY_MEDIUM:
                    IGNIS_CORE_WARN("GL {} [{}] #{}: {}", tipo, sorgente, id, testo);
                    break;

                case GL_DEBUG_SEVERITY_LOW:
                    IGNIS_CORE_WARN("GL {} [{}] #{} (bassa): {}", tipo, sorgente, id, testo);
                    break;

                default:
                    // NOTIFICATION. Oggi non arriva mai qui: la filtriamo a livello
                    // di GL in InitGLDebugOutput. Il ramo resta perché il giorno che
                    // servisse riaccenderla basta togliere UNA riga là sotto.
                    IGNIS_CORE_TRACE("GL {} [{}] #{}: {}", tipo, sorgente, id, testo);
                    break;
            }
        }
    }

    void InitGLDebugOutput()
    {
#if defined(IGNIS_DEBUG)
        // Il contesto di debug si CHIEDE (glfwWindowHint in Window.cpp), non si
        // ottiene: il driver può ignorare la richiesta. Verifichiamo il risultato
        // invece di fidarci dell'intenzione — stessa regola dei file di build.
        GLint flags = 0;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) == 0)
        {
            IGNIS_CORE_WARN("Contesto di debug OpenGL NON ottenuto: il driver ha ignorato "
                            "GLFW_CONTEXT_DEBUG. Gli errori GL resteranno muti.");
            return;
        }

        // Condizione del mondo reale, non invariante: se il driver non espone la
        // funzione, chiamarla sarebbe un salto a nullptr.
        if (!glDebugMessageCallback || !glDebugMessageControl)
        {
            IGNIS_CORE_WARN("Contesto di debug ottenuto, ma glDebugMessageCallback/Control "
                            "non sono state caricate da GLAD. Diagnostica non attiva.");
            return;
        }

        glEnable(GL_DEBUG_OUTPUT);

        // SYNCHRONOUS è metà del valore di questa feature: senza, il driver può
        // accodare i messaggi e la callback scatta quando la chiamata colpevole è
        // già uscita dallo stack. Con, il breakpoint cade sulla riga giusta.
        // Costa prestazioni, ed è il motivo per cui tutto questo è solo in Debug.
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        glDebugMessageCallback(GLDebugCallback, nullptr);

        // Prima tutto acceso...
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        // ...poi via le NOTIFICATION: su NVIDIA sono un fiume ("Buffer detailed info:
        // will use VIDEO memory as the source for buffer object operations") a ogni
        // allocazione, e annegherebbero i TRACE degli eventi. Togli QUESTA riga per
        // riaccenderle: il ramo nella callback c'è già.
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
                              0, nullptr, GL_FALSE);

        IGNIS_CORE_INFO("Debug output OpenGL attivo (sincrono; NOTIFICATION filtrate).");

        // glEnable(GL_TEXTURE_2D);   // VERIFICA task 12 — da togliereS
#else
        IGNIS_CORE_INFO("Debug output OpenGL non attivo: build Release.");
#endif
    }
}
