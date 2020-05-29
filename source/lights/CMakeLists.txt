set(LIGHT_SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/light.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ambient.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/directional.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/point_light.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ambient_occlusion.cpp
)
set(POLY_SOURCE_LIGHT_LIST ${LIGHT_SOURCE} PARENT_SCOPE)
target_sources(raytracer PRIVATE "${LIGHT_SOURCE}")