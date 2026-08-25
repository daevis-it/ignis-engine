#include "Ignis/Core/Input.h"
#include "Ignis/Core/Application.h"
#include "Ignis/Core/Window.h"

#include <GLFW/glfw3.h>

namespace Ignis
{
    // ══════════════════════════════════════════════════════════════════════════
    //  I nostri KeyCode/MouseCode hanno gli stessi valori numerici di GLFW, quindi
    //  la conversione è un cast e non una tabella di 120 righe da tenere allineata.
    //
    //  Questi static_assert sono il prezzo di quella scorciatoia, ed è un prezzo
    //  che vale la pena pagare: se un aggiornamento di GLFW cambiasse anche un solo
    //  valore, il progetto SMETTE DI COMPILARE. Senza, un tasto smetterebbe
    //  semplicemente di funzionare, e nessuno saprebbe perché.
    //
    //  Non li verifichiamo tutti: bastano i confini di ogni blocco della tabella
    //  (ASCII, funzione, tastierino, modificatori), perché GLFW li assegna in
    //  sequenza dentro ciascun blocco.
    // ══════════════════════════════════════════════════════════════════════════
    static_assert(static_cast<int>(KeyCode::Space)        == GLFW_KEY_SPACE);
    static_assert(static_cast<int>(KeyCode::A)            == GLFW_KEY_A);
    static_assert(static_cast<int>(KeyCode::Z)            == GLFW_KEY_Z);
    static_assert(static_cast<int>(KeyCode::D0)           == GLFW_KEY_0);
    static_assert(static_cast<int>(KeyCode::D9)           == GLFW_KEY_9);
    static_assert(static_cast<int>(KeyCode::Escape)       == GLFW_KEY_ESCAPE);
    static_assert(static_cast<int>(KeyCode::Enter)        == GLFW_KEY_ENTER);
    static_assert(static_cast<int>(KeyCode::F1)           == GLFW_KEY_F1);
    static_assert(static_cast<int>(KeyCode::F25)          == GLFW_KEY_F25);
    static_assert(static_cast<int>(KeyCode::KP0)          == GLFW_KEY_KP_0);
    static_assert(static_cast<int>(KeyCode::KPEqual)      == GLFW_KEY_KP_EQUAL);
    static_assert(static_cast<int>(KeyCode::LeftShift)    == GLFW_KEY_LEFT_SHIFT);
    static_assert(static_cast<int>(KeyCode::RightSuper)   == GLFW_KEY_RIGHT_SUPER);
    static_assert(static_cast<int>(KeyCode::Menu)         == GLFW_KEY_MENU);

    static_assert(static_cast<int>(MouseCode::ButtonLeft)   == GLFW_MOUSE_BUTTON_LEFT);
    static_assert(static_cast<int>(MouseCode::ButtonRight)  == GLFW_MOUSE_BUTTON_RIGHT);
    static_assert(static_cast<int>(MouseCode::ButtonMiddle) == GLFW_MOUSE_BUTTON_MIDDLE);
    static_assert(static_cast<int>(MouseCode::Button7)      == GLFW_MOUSE_BUTTON_8);

    bool Input::IsKeyPressed(KeyCode key)
    {
        auto* window = Application::Get().GetWindow().GetNativeWindow();
        const int state = glfwGetKey(window, static_cast<int>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::IsMouseButtonPressed(MouseCode button)
    {
        auto* window = Application::Get().GetWindow().GetNativeWindow();
        return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
    }

    std::pair<float, float> Input::GetMousePosition()
    {
        auto* window = Application::Get().GetWindow().GetNativeWindow();
        double xpos = 0.0, ypos = 0.0;
        glfwGetCursorPos(window, &xpos, &ypos);
        return { static_cast<float>(xpos), static_cast<float>(ypos) };
    }

    float Input::GetMouseX() { return GetMousePosition().first;  }
    float Input::GetMouseY() { return GetMousePosition().second; }
}
