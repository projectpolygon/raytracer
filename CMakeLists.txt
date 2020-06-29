cmake_minimum_required(VERSION 3.12)
project(polygon_raytracer LANGUAGES CXX C)

# --------------------
# Set root
# --------------------
set(POLYGON_ROOT "${PROJECT_SOURCE_DIR}")

# --------------------
# Fetch atlas
# --------------------
include(FetchContent)
FetchContent_Declare(
    atlas
    GIT_REPOSITORY https://github.com/marovira/atlas.git
    )
FetchContent_Populate(atlas)
add_subdirectory(${atlas_SOURCE_DIR} ${atlas_BINARY_DIR})

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.7.3)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

set(POLYGON_INCLUDE "${POLYGON_ROOT}/assignment.hpp")
set(POLYGON_SOURCE "${POLYGON_ROOT}/source/main.cpp")

# --------------------
# Configure the "path" header so files in the program can be placed accordingly
# --------------------
set(PATH_INCLUDE "${POLYGON_ROOT}/include/utilities/paths.hpp")
configure_file("${POLYGON_ROOT}/include/utilities/paths.hpp.in" ${PATH_INCLUDE})
list(APPEND POLYGON_INCLUDE ${PATH_INCLUDE})

# --------------------
# Add the include directory so files can be found
# --------------------
include_directories(${POLYGON_ROOT}/include)

# --------------------
# Create executable target
# --------------------
add_executable(raytracer "${POLYGON_SOURCE}")

# --------------------
# Dive into the source directory to collect source files
# --------------------
add_subdirectory(source)
add_subdirectory(include)

# --------------------
# Update groupings for include and source
# --------------------
source_group("include" FILES  ${POLYGON_INCLUDE})
source_group("include\\BRDFs" FILES ${POLY_INCLUDE_BRDF_GROUP})
source_group("include\\BTDFs" FILES ${POLY_INCLUDE_BTDF_GROUP})
source_group("include\\cameras" FILES ${POLY_INCLUDE_CAMERA_GROUP})
source_group("include\\lights" FILES ${POLY_INCLUDE_LIGHT_GROUP})
source_group("include\\materials" FILES ${POLY_INCLUDE_MATERIAL_GROUP})
source_group("include\\objects" FILES ${POLY_INCLUDE_OBJECT_GROUP})
source_group("include\\samplers" FILES ${POLY_INCLUDE_SAMPLER_GROUP})
source_group("include\\structures" FILES ${POLY_INCLUDE_STRUCTURE_GROUP})
source_group("include\\textures" FILES ${POLY_INCLUDE_TEXTURE_GROUP})
source_group("include\\tracers" FILES ${POLY_INCLUDE_TRACER_GROUP})
source_group("include\\utilities" FILES ${POLY_INCLUDE_UTILITY_GROUP})

source_group("source" FILES  ${POLYGON_SOURCE})
source_group("source\\BRDFs" FILES ${POLY_SOURCE_BRDF_GROUP})
source_group("source\\BTDFs" FILES ${POLY_SOURCE_BTDF_GROUP})
source_group("source\\cameras" FILES ${POLY_SOURCE_CAMERA_GROUP})
source_group("source\\lights" FILES ${POLY_SOURCE_LIGHT_GROUP})
source_group("source\\materials" FILES ${POLY_SOURCE_MATERIAL_GROUP})
source_group("source\\objects" FILES ${POLY_SOURCE_OBJECT_GROUP})
source_group("source\\samplers" FILES ${POLY_SOURCE_SAMPLER_GROUP})
source_group("source\\structures" FILES ${POLY_SOURCE_STRUCTURE_GROUP})
source_group("source\\textures" FILES ${POLY_SOURCE_TEXTURE_GROUP})
source_group("source\\tracers" FILES ${POLY_SOURCE_TRACER_GROUP})
source_group("source\\utilities" FILES ${POLY_SOURCE_UTILITY_GROUP})
source_group("source\\integrators" FILES ${POLY_SOURCE_INTEGRATOR_GROUP})

# --------------------
# Link against Atlas
# --------------------
target_link_libraries(raytracer PUBLIC atlas::atlas nlohmann_json::nlohmann_json)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()
