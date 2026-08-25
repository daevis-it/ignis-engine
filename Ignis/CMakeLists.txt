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

# IGNIS_DEBUG viene da QUI, non da NDEBUG. NDEBUG è un dettaglio di come CMake
# configura la Release; un flag nostro ci lascia liberi di fare un giorno una
# "Debug ottimizzata" o una "Release con assert" senza combattere con lo standard.
# PUBLIC perché IGNIS_ASSERT vive in un header pubblico: anche i client ne dipendono.
target_compile_definitions(Ignis PUBLIC $<$<CONFIG:Debug>:IGNIS_DEBUG>)

# Precompiled header, solo per l'engine (editor e Sandbox sono un file ciascuno).
# CMake genera da sé l'unità di traduzione necessaria: non serve un ignispch.cpp.
target_precompile_headers(Ignis PRIVATE Private/ignispch.h)

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
