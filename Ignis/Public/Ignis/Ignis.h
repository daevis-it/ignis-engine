#pragma once

// ══ Header ombrello ══
//
// È l'unica cosa che un client (editor o gioco) deve includere per usare Ignis.
// NON include EntryPoint.h: quello va incluso una volta sola, nel file che
// definisce CreateApplication(), altrimenti si finisce con più main() nel binario.

#include "Ignis/Core/Base.h"
#include "Ignis/Core/Logger.h"
#include "Ignis/Core/Application.h"
#include "Ignis/Core/Input.h"
#include "Ignis/Core/Layer.h"
#include "Ignis/Core/Timestep.h"
#include "Ignis/Core/LayerStack.h"
#include "Ignis/Core/KeyCodes.h"
#include "Ignis/Core/MouseCodes.h"
#include "Ignis/Core/Paths.h"
#include "Ignis/Core/Window.h"

#include "Ignis/Renderer/RenderCommand.h"

#include "Ignis/Events/Event.h"
#include "Ignis/Events/ApplicationEvent.h"
#include "Ignis/Events/KeyEvent.h"
#include "Ignis/Events/MouseEvent.h"
