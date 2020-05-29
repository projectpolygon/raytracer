set(BTDF_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/perfect_transmitter.cpp
)
set(POLY_SOURCE_BTDF_LIST ${BTDF_SOURCE} PARENT_SCOPE)
target_sources(raytracer PRIVATE "${BTDF_SOURCE}")