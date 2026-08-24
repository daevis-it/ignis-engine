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
