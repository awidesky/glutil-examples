cmake_minimum_required(VERSION 3.20)

project(glutil-example LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# external/
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external" CACHE PATH "Directory for external dependencies")
set(GDM_EXTERNAL_DIR "${EXTERNAL_DIR}" CACHE PATH "Directory where GDM external dependencies are stored" FORCE)
file(MAKE_DIRECTORY "${EXTERNAL_DIR}")

# GDM options
set(GDM_USE_GLUTIL ON CACHE BOOL "" FORCE)
set(GDM_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GDM_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(GDM_WINDOW_BACKEND "glfw" CACHE STRING "" FORCE)
set(GDM_GL_LOADER "glad" CACHE STRING "" FORCE)
set(GDM_USE_GLM ON CACHE BOOL "" FORCE)

set(GLFW_VERSION "3.2.0" CACHE STRING "" FORCE)
set(GLM_VERSION "1.0.1" CACHE STRING "" FORCE)
set(GLAD_VERSION "2.0.8" CACHE STRING "" FORCE)

set(GDM_GLAD_EXTENSION
    "GL_ARB_debug_output;GL_EXT_debug_label;GL_EXT_debug_marker;GL_EXT_texture_compression_s3tc;GL_KHR_debug;GL_EXT_texture_filter_anisotropic"
    CACHE STRING "" FORCE
)

# Fetch gdm
set(GDM_DIR "${EXTERNAL_DIR}/gdm")
include(FetchContent)

FetchContent_Declare(
    gdm
    GIT_REPOSITORY https://github.com/awidesky/gdm.git
    GIT_TAG debug
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
    GIT_REMOTE_UPDATE_STRATEGY CHECKOUT
    SOURCE_DIR "${GDM_DIR}"
)

FetchContent_MakeAvailable(gdm)

set(ASSET_DIR "${CMAKE_SOURCE_DIR}/assets")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generatedHeader")
configure_file(
    "${CMAKE_SOURCE_DIR}/src/config.hpp.in"
    "${CMAKE_BINARY_DIR}/generatedHeader/config.hpp"
    @ONLY
)


# Example targets
function(add_gdm_example TARGET_NAME)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCES INCLUDE_DIRS)

    cmake_parse_arguments(ARG
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    add_executable(${TARGET_NAME} ${ARG_SOURCES})

    target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_BINARY_DIR}/generatedHeader"
        ${ARG_INCLUDE_DIRS}
    )
    target_link_libraries(${TARGET_NAME} PRIVATE
        gdm::deps
        gdm::glutil
    )

    target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)

    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4
            /utf-8
        )
    else()
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()
endfunction()


add_gdm_example(01_ManyCubes
    SOURCES
        src/manyCubes.cpp

        shader/manyCubes.vert
        shader/manyCubes.frag
)
add_gdm_example(02_Excavator_bad
    SOURCES
        src/excavator_bad_utils.hpp
        src/excavator_bad.cpp

        shader/excvator.vert
        shader/excvator.frag
)
if(MSVC)
    set_source_files_properties(
        src/excavator_bad_utils.hpp
        PROPERTIES COMPILE_OPTIONS "/w"
    )
else()
    set_source_files_properties(
        src/excavator_bad_utils.hpp
        PROPERTIES COMPILE_OPTIONS "-w"
    )
endif()
add_gdm_example(03_Excavator_good
    SOURCES
        src/excavator_good.cpp

        shader/excvator.vert
        shader/excvator.frag
)
add_gdm_example(04_AIMLAB
    SOURCES
        src/aimlab/AIMLAB_main.cpp
        src/aimlab/gameloop.hpp
        src/aimlab/camera.hpp
        src/aimlab/engine.hpp
        src/aimlab/component.hpp
        src/aimlab/component.cpp
        src/aimlab/physics.hpp
        src/aimlab/mesh.hpp
        src/aimlab/manager.hpp
        src/aimlab/score.hpp
	
        shader/aimlab.vert
        shader/aimlab.frag

    INCLUDE_DIRS
        src/aimlab
        ${EXTERNAL_DIR}/header_only
)

source_group("Shaders"
    REGULAR_EXPRESSION ".*\\.(frag|vert)$"
)

include("cmake/resources.cmake")

fetch_example_asset(basic)
fetch_example_asset(excavator)
fetch_example_asset(num)
fetch_example_asset(AIMLAB)
fetch_example_asset(crosshair)
fetch_example_asset(ak47)

download_repo_file(
    REPOSITORY https://github.com/mackron/miniaudio
    TAG 0.11.25
    FILE miniaudio.h
    DESTINATION_DIR "${EXTERNAL_DIR}/header_only"
)
 