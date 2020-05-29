set(BRDF_INCLUDE 
	${CMAKE_CURRENT_SOURCE_DIR}/glossy_specular.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/glossy_specular.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/lambertian.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/SV_lambertian.hpp 
	${CMAKE_CURRENT_SOURCE_DIR}/perfect_specular.hpp
)
set(POLY_INCLUDE_BRDF_LIST ${BRDF_INCLUDE} PARENT_SCOPE)