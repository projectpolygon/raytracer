set(BRDF_SOURCE 
    ${CMAKE_CURRENT_SOURCE_DIR}/glossy_specular.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/lambertian.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/SV_lambertian.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/perfect_specular.cpp
)
set(POLY_SOURCE_BRDF_LIST ${BRDF_SOURCE} PARENT_SCOPE)
target_sources(raytracer PRIVATE "${BRDF_SOURCE}")