set(STRUCTURE_INCLUDE
	${CMAKE_CURRENT_INCLUDE_DIR}/world.hpp 
	${CMAKE_CURRENT_INCLUDE_DIR}/bounds.hpp
	${CMAKE_CURRENT_INCLUDE_DIR}/view_plane.hpp 
	${CMAKE_CURRENT_INCLUDE_DIR}/KDTree.hpp
	${CMAKE_CURRENT_INCLUDE_DIR}/scene_slab.hpp
	${CMAKE_CURRENT_INCLUDE_DIR}/surface_interaction.hpp
)
set(POLY_INCLUDE_STRUCTURE_LIST ${STRUCTURE_INCLUDE} PARENT_SCOPE)