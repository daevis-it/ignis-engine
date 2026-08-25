#pragma once

#include <cstdint>

namespace Ignis
{
    // Generato dai #define di GLFW/glfw3.h (GLFW 3.4). Vedi KeyCodes.h per il
    // perché i valori coincidono con quelli di GLFW.
    enum class MouseCode : std::uint8_t
    {
        Button0 = 0,
        Button1 = 1,
        Button2 = 2,
        Button3 = 3,
        Button4 = 4,
        Button5 = 5,
        Button6 = 6,
        Button7 = 7,

        // Alias per i tre pulsanti che hanno un nome nella vita reale.
        ButtonLeft   = Button0,
        ButtonRight  = Button1,
        ButtonMiddle = Button2
    };
}
