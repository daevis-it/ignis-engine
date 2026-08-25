#pragma once

#include "Ignis/Core/KeyCodes.h"
#include "Ignis/Core/MouseCodes.h"

#include <utility>

namespace Ignis
{
    // Interfaccia statica sullo stato dei dispositivi.
    //
    // Non duplica il sistema di eventi, lo completa: gli eventi dicono COS'È
    // SUCCESSO (il salto si fa lì), l'Input dice COM'È ADESSO (il movimento
    // continuo si fa qui). Servono entrambi, e chiedere lo stato a un evento è
    // il modo classico di ottenere controlli che "saltano".
    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key);

        static bool IsMouseButtonPressed(MouseCode button);
        static std::pair<float, float> GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };
}
