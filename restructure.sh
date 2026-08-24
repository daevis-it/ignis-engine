#!/usr/bin/env bash
#
# Ignis Engine — Task 01a/01b/01c
# Ristrutturazione dell'albero in Public/Private, build a tre target, dipendenze FetchContent.
#
# Lo script è IDEMPOTENTE nei limiti del ragionevole: se una destinazione esiste già la salta.
# Non cancella niente che non sia rigenerabile (il vendor ImGui torna da FetchContent).
#
# Uso:  cd /home/daevis/Code/ignis-engine && bash restructure.sh
#
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

info()  { printf '\033[1;36m[ IGNIS ]\033[0m %s\n' "$*"; }
warn()  { printf '\033[1;33m[ ATTENZIONE ]\033[0m %s\n' "$*"; }
die()   { printf '\033[1;31m[ ERRORE ]\033[0m %s\n' "$*" >&2; exit 1; }

# --------------------------------------------------------------------------------------
# 0. Guardie. Meglio fermarsi ora che a metà.
# --------------------------------------------------------------------------------------
[ -d .git ]            || die "Non sono nella radice di un repo git. Sono in: $ROOT"
[ -f CMakeLists.txt ]  || die "Non trovo CMakeLists.txt: sicuro che questa sia la cartella di Ignis?"

if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    die "Ci sono modifiche non committate. Committa o stasha prima: questo script sposta ogni file del repo."
fi

info "Repo pulito. Parto dalla ristrutturazione."

# git mv che non esplode se la sorgente non c'è (script rilanciabile)
gmv() {
    local src="$1" dst="$2"
    if [ -e "$src" ]; then
        mkdir -p "$(dirname "$dst")"
        git mv -f "$src" "$dst"
        printf '  %s  ->  %s\n' "$src" "$dst"
    fi
}

# --------------------------------------------------------------------------------------
# 1. L'albero
# --------------------------------------------------------------------------------------
info "Creo l'albero..."
mkdir -p cmake
mkdir -p Ignis/Public/Ignis/{Core,Events,ImGui}
mkdir -p Ignis/Private/Ignis/{Core,ImGui}
mkdir -p Ignis/vendor/glad/include/{glad,KHR}
mkdir -p Ignis/vendor/glad/src
mkdir -p IgnisEditor/Private
mkdir -p Sandbox/Private

# --------------------------------------------------------------------------------------
# 2. Header pubblici dell'engine
# --------------------------------------------------------------------------------------
info "Sposto gli header pubblici..."
gmv include/Ignis/Core/Application.h          Ignis/Public/Ignis/Core/Application.h
gmv include/Ignis/Core/EntryPoint.h           Ignis/Public/Ignis/Core/EntryPoint.h
gmv include/Ignis/Core/Input.h                Ignis/Public/Ignis/Core/Input.h
gmv include/Ignis/Core/Logger.h               Ignis/Public/Ignis/Core/Logger.h
gmv include/Ignis/Core/Window.h               Ignis/Public/Ignis/Core/Window.h
gmv include/Ignis/Events/Event.h              Ignis/Public/Ignis/Events/Event.h
gmv include/Ignis/Events/ApplicationEvent.h   Ignis/Public/Ignis/Events/ApplicationEvent.h
gmv include/Ignis/Events/KeyEvent.h           Ignis/Public/Ignis/Events/KeyEvent.h
gmv include/Ignis/Events/MouseEvent.h         Ignis/Public/Ignis/Events/MouseEvent.h
gmv include/Ignis/ImGui/ImGuiLayer.h          Ignis/Public/Ignis/ImGui/ImGuiLayer.h

# --------------------------------------------------------------------------------------
# 3. Sorgenti privati dell'engine
# --------------------------------------------------------------------------------------
info "Sposto i sorgenti privati..."
gmv src/Ignis/Core/Application.cpp            Ignis/Private/Ignis/Core/Application.cpp
gmv src/Ignis/Core/Input.cpp                  Ignis/Private/Ignis/Core/Input.cpp
gmv src/Ignis/Core/Window.cpp                 Ignis/Private/Ignis/Core/Window.cpp
gmv src/Ignis/ImGui/ImGuiLayer.cpp            Ignis/Private/Ignis/ImGui/ImGuiLayer.cpp

# --------------------------------------------------------------------------------------
# 4. Il client editor
# --------------------------------------------------------------------------------------
info "Sposto il client..."
gmv src/Ignis/EditorApp.cpp                   IgnisEditor/Private/EditorApp.cpp

# --------------------------------------------------------------------------------------
# 5. GLAD — codice generato, resta versionato (decisione D3)
# --------------------------------------------------------------------------------------
info "Sposto GLAD..."
gmv include/vendor/glad/glad.h                Ignis/vendor/glad/include/glad/glad.h
gmv include/vendor/KHR/khrplatform.h          Ignis/vendor/glad/include/KHR/khrplatform.h
gmv src/vendor/glad.c                         Ignis/vendor/glad/src/glad.c

# --------------------------------------------------------------------------------------
# 6. Via il vendor ImGui: da qui in poi arriva da FetchContent (decisione D2)
# --------------------------------------------------------------------------------------
info "Rimuovo il vendor ImGui committato (~4 MB, torna da FetchContent)..."
[ -d include/vendor/imgui ] && git rm -r -q --ignore-unmatch include/vendor/imgui || true
[ -d src/vendor/imgui ]     && git rm -r -q --ignore-unmatch src/vendor/imgui     || true

# Le vecchie cartelle, se svuotate, se ne vanno
find include src -type d -empty -delete 2>/dev/null || true

# --------------------------------------------------------------------------------------
# 7. Gli include di ImGui cambiano prefisso
#    Prima:  <imgui/imgui.h>          (cartella vendor nostra)
#    Ora:    <imgui.h>                (include dir del target imgui di FetchContent)
# --------------------------------------------------------------------------------------
info "Aggiorno gli include di ImGui..."
while IFS= read -r -d '' f; do
    sed -i \
        -e 's#<imgui/imgui\.h>#<imgui.h>#g' \
        -e 's#<imgui/imgui_impl_glfw\.h>#<imgui_impl_glfw.h>#g' \
        -e 's#<imgui/imgui_impl_opengl3\.h>#<imgui_impl_opengl3.h>#g' \
        -e 's#<imgui/imgui_internal\.h>#<imgui_internal.h>#g' \
        "$f"
done < <(find Ignis IgnisEditor Sandbox -type f \( -name '*.cpp' -o -name '*.h' \) -print0)

# --------------------------------------------------------------------------------------
# 8. File nuovi
# --------------------------------------------------------------------------------------
info "Scrivo i file di build..."

# ---------- cmake/Dependencies.cmake ----------
cat > cmake/Dependencies.cmake <<'EOF'
# Dipendenze esterne — decisione D2: FetchContent, niente installazioni a sistema.
# Il primo configure scarica e compila (qualche minuto); i successivi usano la cache.

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# --------------------------------------------------------------------- GLFW
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

# Su Linux GLFW 3.4 compila per default SIA X11 SIA Wayland, e il backend Wayland
# pretende wayland-scanner, libwayland-dev e libxkbcommon-dev già in fase di configure:
# se mancano, la configure muore con "Failed to find wayland-scanner".
# Mint 22.3 / Cinnamon gira su X11, quindi teniamo Wayland spento finché non serve.
if(UNIX AND NOT APPLE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_X11     ON  CACHE BOOL "" FORCE)
endif()
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
    GIT_SHALLOW    TRUE
)

# --------------------------------------------------------------------- GLM
# Non serve ancora a nessuno: servirà dal Renderer 2D in poi. La dichiariamo ora
# perché aggiungere una dipendenza a build funzionante è un cambio isolato.
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

# --------------------------------------------------------------------- Dear ImGui
# 'docking' è un BRANCH, non un tag: si muove sotto i piedi. Pinniamo il commit
# esatto su cui questa build è stata verificata, così fra sei mesi il repo compila
# ancora con lo stesso ImGui invece che con quello del giorno.
#
# Per aggiornare in futuro, deliberatamente:
#   1. rimetti GIT_TAG docking, riconfigura
#   2. git -C build/<preset>/_deps/imgui-src rev-parse HEAD
#   3. rimetti lo SHA qui sotto e verifica che compili ancora
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        fd13a1e8923a0a7077b404fc36fd063b25a0c0b5  # branch docking @ 2026-08-19
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(glfw glm imgui)

# ImGui non fornisce un CMakeLists: il target lo costruiamo noi, scegliendo
# esplicitamente i due backend che ci servono (GLFW + OpenGL3).
if(NOT TARGET imgui)
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw)
endif()
EOF

# ---------- CMakeLists.txt (root) ----------
cat > CMakeLists.txt <<'EOF'
cmake_minimum_required(VERSION 3.20)
project(IgnisEngine LANGUAGES C CXX)

# Decisione D5: C++20, non 23. std::format esiste su GCC 13 e su MSVC 2022+;
# del C++23 i due compilatori supportano sottoinsiemi diversi.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # niente estensioni GNU: portabilità verso MSVC

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Utile a clangd / CLion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/Dependencies.cmake)

add_subdirectory(Ignis)
add_subdirectory(IgnisEditor)
add_subdirectory(Sandbox)
EOF

# ---------- Ignis/vendor/glad/CMakeLists.txt ----------
cat > Ignis/vendor/glad/CMakeLists.txt <<'EOF'
# GLAD è codice GENERATO, non una libreria che evolve (decisione D3).
# Target separato così i warning severi dell'engine non si applicano a lui.
#
# Generato su https://glad.dav1d.de/ con:
#   Language: C/C++   |   Specification: OpenGL
#   API: gl 3.3       |   Profile: Core
#   Extensions: nessuna   |   Options: Generate a loader
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL ${CMAKE_DL_LIBS})
EOF

# ---------- Ignis/CMakeLists.txt ----------
cat > Ignis/CMakeLists.txt <<'EOF'
# ══ Ignis — la libreria. Non conosce né l'editor né i giochi. ══

add_subdirectory(vendor/glad)

# CONFIGURE_DEPENDS: CMake ricontrolla i glob a ogni build, così aggiungere un file
# non richiede più il "Reload CMake Project" a mano (era la trappola nel troubleshooting).
file(GLOB_RECURSE IGNIS_PUBLIC_HEADERS CONFIGURE_DEPENDS "Public/*.h")
file(GLOB_RECURSE IGNIS_PRIVATE_FILES  CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

add_library(Ignis STATIC ${IGNIS_PUBLIC_HEADERS} ${IGNIS_PRIVATE_FILES})
add_library(Ignis::Ignis ALIAS Ignis)

# QUI vive il confine Public/Private: PUBLIC è ciò che i client possono includere,
# PRIVATE è visibile solo mentre si compila l'engine. Un client che prova a includere
# un header privato non trova il file: errore di compilazione, non cattiva abitudine.
target_include_directories(Ignis
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Public
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

# PUBLIC e non PRIVATE perché Window.h espone <glad/glad.h> e <GLFW/glfw3.h>.
# È un debito noto — l'header pubblico ridistribuisce GLFW ai client — e si salda al task 05.
target_link_libraries(Ignis PUBLIC glad glfw glm::glm imgui)

if(MSVC)
    # /utf-8          i sorgenti hanno commenti accentati: senza questo MSVC li legge
    #                 nella codepage locale (C4819 e stringhe corrotte)
    # /Zc:__cplusplus altrimenti MSVC riporta __cplusplus fermo al 2003
    # /permissive-    conformance mode: avvicina MSVC a GCC invece del contrario
    target_compile_options(Ignis PUBLIC /utf-8 /Zc:__cplusplus /permissive-)
    target_compile_options(Ignis PRIVATE /W4)
else()
    target_compile_options(Ignis PRIVATE -Wall -Wextra)
endif()
EOF

# ---------- IgnisEditor/CMakeLists.txt ----------
cat > IgnisEditor/CMakeLists.txt <<'EOF'
# ══ IgnisEditor — un client. Linka l'engine come farebbe chiunque altro. ══

file(GLOB_RECURSE EDITOR_FILES CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

add_executable(IgnisEditor ${EDITOR_FILES})
target_include_directories(IgnisEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)
target_link_libraries(IgnisEditor PRIVATE Ignis)

if(MSVC)
    target_compile_options(IgnisEditor PRIVATE /W4)
else()
    target_compile_options(IgnisEditor PRIVATE -Wall -Wextra)
endif()
EOF

# ---------- Sandbox/CMakeLists.txt ----------
cat > Sandbox/CMakeLists.txt <<'EOF'
# ══ Sandbox — il secondo client. ══
#
# Non è un vezzo: è il test che l'engine sia davvero linkabile dall'esterno.
# Con un target solo, un accoppiamento sbagliato resta invisibile perché tanto
# compila lo stesso. Con due client sulla stessa libreria, esce subito.

file(GLOB_RECURSE SANDBOX_FILES CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

add_executable(Sandbox ${SANDBOX_FILES})
target_include_directories(Sandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private)
target_link_libraries(Sandbox PRIVATE Ignis)

if(MSVC)
    target_compile_options(Sandbox PRIVATE /W4)
else()
    target_compile_options(Sandbox PRIVATE -Wall -Wextra)
endif()
EOF

# ---------- CMakePresets.json ----------
cat > CMakePresets.json <<'EOF'
{
  "version": 3,
  "cmakeMinimumRequired": { "major": 3, "minor": 21, "patch": 0 },

  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "binaryDir": "${sourceDir}/build/${presetName}",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },

    {
      "name": "linux-gcc-debug",
      "displayName": "Linux · GCC · Debug",
      "inherits": "base",
      "generator": "Ninja",
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Linux" },
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_C_COMPILER": "gcc",
        "CMAKE_CXX_COMPILER": "g++"
      }
    },
    {
      "name": "linux-gcc-release",
      "displayName": "Linux · GCC · Release",
      "inherits": "linux-gcc-debug",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" }
    },

    {
      "name": "windows-msvc-debug",
      "displayName": "Windows · MSVC · Debug",
      "inherits": "base",
      "generator": "Ninja",
      "condition": { "type": "equals", "lhs": "${hostSystemName}", "rhs": "Windows" },
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_C_COMPILER": "cl",
        "CMAKE_CXX_COMPILER": "cl"
      }
    },
    {
      "name": "windows-msvc-release",
      "displayName": "Windows · MSVC · Release",
      "inherits": "windows-msvc-debug",
      "cacheVariables": { "CMAKE_BUILD_TYPE": "Release" }
    }
  ],

  "buildPresets": [
    { "name": "linux-gcc-debug",     "configurePreset": "linux-gcc-debug" },
    { "name": "linux-gcc-release",   "configurePreset": "linux-gcc-release" },
    { "name": "windows-msvc-debug",  "configurePreset": "windows-msvc-debug" },
    { "name": "windows-msvc-release","configurePreset": "windows-msvc-release" }
  ]
}
EOF

# ---------- Ignis/Public/Ignis/Ignis.h ----------
cat > Ignis/Public/Ignis/Ignis.h <<'EOF'
#pragma once

// ══ Header ombrello ══
//
// È l'unica cosa che un client (editor o gioco) deve includere per usare Ignis.
// NON include EntryPoint.h: quello va incluso una volta sola, nel file che
// definisce CreateApplication(), altrimenti si finisce con più main() nel binario.

#include "Ignis/Core/Logger.h"
#include "Ignis/Core/Application.h"
#include "Ignis/Core/Input.h"
#include "Ignis/Core/Window.h"

#include "Ignis/Events/Event.h"
#include "Ignis/Events/ApplicationEvent.h"
#include "Ignis/Events/KeyEvent.h"
#include "Ignis/Events/MouseEvent.h"
EOF

# ---------- Sandbox/Private/SandboxApp.cpp ----------
cat > Sandbox/Private/SandboxApp.cpp <<'EOF'
#include "Ignis/Ignis.h"
#include "Ignis/Core/EntryPoint.h"

// Il Sandbox è un gioco finto: serve a dimostrare che l'engine si linka da fuori
// usando SOLO gli header pubblici. Se un giorno questo file non compila più mentre
// l'editor sì, vuol dire che abbiamo fatto passare un dettaglio interno nell'API.

class SandboxApplication : public Ignis::Application
{
public:
    SandboxApplication()
    {
        Ignis::Logger::Info("Sandbox avviato: sto usando Ignis come farebbe un gioco vero.");
    }

    ~SandboxApplication() override = default;
};

Ignis::Application* Ignis::CreateApplication()
{
    return new SandboxApplication();
}
EOF

# ---------- .gitignore ----------
cat > .gitignore <<'EOF'
# --- IDE ---
.idea/
.vs/
.vscode/
*.user

# --- Build ---
build/
cmake-build-*/
out/
CMakeUserPresets.json

# --- Artefatti di compilazione ---
*.o
*.obj
*.a
*.lib
*.so
*.dll
*.exe
*.pdb
*.ilk

# --- Runtime ---
imgui.ini
EOF

# --------------------------------------------------------------------------------------
# 9. Pulizia finale
# --------------------------------------------------------------------------------------
info "Pulizia..."
rm -rf cmake-build-debug
git add -A

# --------------------------------------------------------------------------------------
# 10. Resoconto
# --------------------------------------------------------------------------------------
echo
info "Albero risultante:"
git ls-files | grep -v '^Ignis/vendor/glad/' | sed 's/^/  /'
echo
info "GLAD (non elencato sopra): $(git ls-files 'Ignis/vendor/glad/*' | wc -l) file"
echo
info "Fatto. Le modifiche sono in stage, NON committate: guardale con 'git status' prima."
echo
echo "  Prossimo passo:"
echo "    cmake --preset linux-gcc-debug     # la prima volta scarica GLFW/GLM/ImGui"
echo "    cmake --build build/linux-gcc-debug"
echo "    ./build/linux-gcc-debug/bin/IgnisEditor"
echo "    ./build/linux-gcc-debug/bin/Sandbox"
echo
