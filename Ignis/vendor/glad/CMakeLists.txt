# GLAD è codice GENERATO, non una libreria che evolve (decisione D3).
# Target separato così i warning severi dell'engine non si applicano a lui.
#
# Rigenerato il 2026-08-25 per OpenGL 4.5 Core (decisione D8).
# Riproducibile senza passare dal sito web:
#
#   pip install glad==0.1.36
#   glad --profile core --api gl=4.5 --generator c --out-path <dir>
#
# (glad 0.1.x = "glad1", la stessa generazione di prima: <glad/glad.h> e
#  gladLoadGLLoader restano identici, cambia solo il set di funzioni.)
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL ${CMAKE_DL_LIBS})
