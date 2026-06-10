cmake_minimum_required(VERSION 3.20)

project(glutil-example LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# external/
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")
file(MAKE_DIRECTORY "${EXTERNAL_DIR}")

# Download gdm if missing
set(GDM_DIR "${EXTERNAL_DIR}/gdm")

if(NOT EXISTS "${GDM_DIR}/CMakeLists.txt")
    message(STATUS "[${PROJECT_NAME}] Fetching gdm...")

    set(GDM_URL "https://github.com/awidesky/gdm/archive/refs/heads/master.tar.gz")
    set(GDM_ARCHIVE "${CMAKE_BINARY_DIR}/gdm.tar.gz")
    set(GDM_EXTRACT_DIR "${CMAKE_BINARY_DIR}/gdm_extract")

    file(DOWNLOAD
        "${GDM_URL}"
        "${GDM_ARCHIVE}"
        STATUS _dl_status
        LOG _dl_log
    )

    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
        message(FATAL_ERROR
            "Failed to download gdm"
            "Download status: ${_dl_status}, Log : ${_dl_log}")
    endif()

    file(REMOVE_RECURSE "${GDM_EXTRACT_DIR}")

    file(ARCHIVE_EXTRACT
        INPUT "${GDM_ARCHIVE}"
        DESTINATION "${GDM_EXTRACT_DIR}"
    )

    file(GLOB _gdm_dirs
        LIST_DIRECTORIES true
        "${GDM_EXTRACT_DIR}/*"
    )

    list(GET _gdm_dirs 0 _gdm_root)
    file(RENAME "${_gdm_root}" "${GDM_DIR}")
endif()

# GDM options
set(GDM_USE_GLUTIL ON CACHE BOOL "" FORCE)
set(GDM_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GDM_BUILD_TESTS OFF CACHE BOOL "" FORCE)

set(GDM_WINDOW_BACKEND "glfw" CACHE STRING "" FORCE)
set(GDM_GL_LOADER "glad" CACHE STRING "" FORCE)
set(GDM_USE_GLM ON CACHE BOOL "" FORCE)

set(GLFW_VERSION "3.4.0" CACHE STRING "" FORCE)
set(GLM_VERSION "1.0.1" CACHE STRING "" FORCE)
set(GLAD_VERSION "2.0.8" CACHE STRING "" FORCE)

set(GDM_GLAD_EXTENSION
    "GL_ARB_debug_output;GL_EXT_debug_label;GL_EXT_debug_marker;GL_EXT_texture_compression_s3tc;GL_KHR_debug;GL_EXT_texture_filter_anisotropic"
    CACHE STRING "" FORCE
)

add_subdirectory("${GDM_DIR}")

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
        external/header_only
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
    DESTINATION_DIR ${PROJECT_SOURCE_DIR}/external/header_only
)
 