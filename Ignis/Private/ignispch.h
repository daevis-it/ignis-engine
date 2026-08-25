#pragma once

// ══════════════════════════════════════════════════════════════════════════════
//  Precompiled header — solo per il target Ignis (PRIVATE).
//
//  Qui dentro va SOLO ciò che è davvero universale e che non cambia mai:
//  la libreria standard. Niente altro, per due motivi.
//
//  Niente ImGui / GLFW / GLAD: li usano due file su cinque, e infilarli qui
//  significherebbe che ogni file dell'engine si porta dentro OpenGL anche quando
//  non c'entra niente. Quando il Renderer li userà davvero ovunque, si aggiungono
//  ed è una riga.
//
//  Niente Logger.h / Base.h: renderebbe le macro IGNIS_* disponibili ovunque senza
//  include, ma nasconderebbe le dipendenze — apri un .cpp, vedi IGNIS_CORE_ERROR e
//  non sai da dove arriva. IL PCH È UN'OTTIMIZZAZIONE, NON UN MODO PER SMETTERE DI
//  SCRIVERE GLI INCLUDE.
// ══════════════════════════════════════════════════════════════════════════════

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
