#pragma once

#include <glm/glm.hpp>

#include <cstdint>

namespace Ignis
{
    // ══════════════════════════════════════════════════════════════════════════
    //  I comandi elementari verso la GPU.
    //
    //  È lo strato più sottile possibile sopra OpenGL: una funzione per verbo,
    //  nessuno stato, nessuna decisione. Non è l'astrazione multi-API di D7 — è
    //  D11/D17 applicati alle poche chiamate che non appartengono a nessuna classe
    //  wrapper, perché non possiedono una risorsa.
    //
    //  NESSUN TIPO OPENGL NELLE FIRME, ed è la parte che conta: chi include questo
    //  header non vede glad, non lo linka, e non sa quale API grafica c'è sotto.
    //  Le GLint/GLsizei vivono tutte nel .cpp, in Private/Ignis/Renderer/.
    //
    //  Classe statica come Input: stesso vocabolario per la stessa forma. Un
    //  namespace di funzioni libere farebbe la stessa cosa, ma due modi di scrivere
    //  "servizio globale dell'engine" nello stesso progetto sono uno di troppo.
    // ══════════════════════════════════════════════════════════════════════════
    class RenderCommand
    {
    public:
        // Coordinate e dimensioni sono PIXEL del framebuffer, non screen coordinates:
        // su HiDPI i due numeri divergono, e a glViewport servono i pixel. È lo stesso
        // avvertimento che sta su Window::GetWidth e su WindowResizeEvent, ed è la
        // ragione per cui il resize passa da glfwSetFramebufferSizeCallback.
        static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        // glm::vec4 e non quattro float: tutta l'API del renderer parlerà glm —
        // posizioni, colori, matrici — e glm è già PUBLIC nella build per decisione.
        // Tenere fuori glm da questa firma sola non eviterebbe niente, rimanderebbe
        // di tre task e intanto lascerebbe due vocabolari per la stessa cosa.
        static void SetClearColor(const glm::vec4& colore);

        // Oggi pulisce SOLO il color buffer. Il depth entrerà quando ci sarà una
        // profondità da azzerare — z-order al task 19, o il 3D in Fase 7. Aggiungere
        // GL_DEPTH_BUFFER_BIT adesso, senza depth buffer nel framebuffer, sarebbe
        // arredo: una riga che non fa niente e che sembra fare qualcosa.
        static void Clear();
    };
}
