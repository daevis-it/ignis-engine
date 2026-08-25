# ══ Ignis — la libreria. Non conosce né l'editor né i giochi. ══

add_subdirectory(vendor/glad)

# CONFIGURE_DEPENDS: CMake ricontrolla i glob a ogni build, così aggiungere un file
# non richiede più il "Reload CMake Project" a mano (era la trappola nel troubleshooting).
file(GLOB_RECURSE IGNIS_PUBLIC_HEADERS CONFIGURE_DEPENDS "Public/*.h")
file(GLOB_RECURSE IGNIS_PRIVATE_FILES  CONFIGURE_DEPENDS "Private/*.cpp" "Private/*.h")

# ── Guardia: nessun file può esistere con lo STESSO percorso relativo in Public e
# in Private. Entrambe le cartelle mappano su "Ignis/...", quindi un omonimo sarebbe
# ambiguo: vincerebbe silenziosamente quello in Public, perché è il primo negli
# include path, e la versione privata verrebbe ignorata senza un solo messaggio.
# È il tipo di errore che si manifesta come "ho modificato il file e non cambia niente".
foreach(_ignis_pub IN LISTS IGNIS_PUBLIC_HEADERS)
    file(RELATIVE_PATH _ignis_rel "${CMAKE_CURRENT_SOURCE_DIR}/Public" "${_ignis_pub}")
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Private/${_ignis_rel}")
        message(FATAL_ERROR
            "Ignis: '${_ignis_rel}' esiste sia in Public/ sia in Private/. "
            "L'include \"${_ignis_rel}\" sarebbe ambiguo e vincerebbe la versione "
            "pubblica in silenzio. Rinominane una delle due.")
    endif()
endforeach()

add_library(Ignis STATIC ${IGNIS_PUBLIC_HEADERS} ${IGNIS_PRIVATE_FILES})
add_library(Ignis::Ignis ALIAS Ignis)

# QUI vive il confine Public/Private: PUBLIC è ciò che i client possono includere,
# PRIVATE è visibile solo mentre si compila l'engine. Un client che prova a includere
# un header privato non trova il file: errore di compilazione, non cattiva abitudine.
target_include_directories(Ignis
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Public
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

# PRIVATE, dal task 05c: nessun header pubblico include più glad, GLFW o ImGui,
# quindi i client non li vedono e non li linkano. Un gioco costruito su Ignis non
# sa — e non deve sapere — quale libreria di finestre c'è sotto.
# glm resta PUBLIC: la matematica è parte dell'API che i giochi useranno.
target_link_libraries(Ignis
    PRIVATE glad glfw imgui
    PUBLIC  glm::glm
)

# ── Configurazione ──
# IGNIS_DEBUG / IGNIS_RELEASE vengono da QUI, non da NDEBUG: un flag nostro lascia
# libera la possibilità di una "Debug ottimizzata" o di una "Release con assert".
# PUBLIC perché IGNIS_ASSERT vive in un header pubblico: anche i client ne dipendono.
#
# ENTRAMBE le configurazioni definiscono qualcosa, di proposito: Base.h fa #error se
# non ne trova nessuna. Il 2026-08-25 questa riga è stata persa per tre task, e il
# risultato è stato assert disattivati e TRACE spariti SENZA UN SEGNALE. Ora
# perderla di nuovo significa non compilare.
target_compile_definitions(Ignis PUBLIC
    $<$<CONFIG:Debug>:IGNIS_DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:IGNIS_RELEASE>
)

# Precompiled header, solo per l'engine (editor e Sandbox sono pochi file).
# CMake genera da sé l'unità di traduzione necessaria: non serve un ignispch.cpp.
target_precompile_headers(Ignis PRIVATE Private/ignispch.h)

# Il PCH non può essere verificato da un #error: lo dichiariamo al configure, così
# se un giorno sparisse si vedrebbe nell'output invece che nei tempi di build.
get_target_property(_ignis_pch Ignis PRECOMPILE_HEADERS)
message(STATUS "Ignis: precompiled header = ${_ignis_pch}")

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
