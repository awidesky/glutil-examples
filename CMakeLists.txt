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

    set(GDM_URL "https://github.com/awidesky/gdm/archive/refs/heads/label.tar.gz")
    set(GDM_ARCHIVE "${CMAKE_BINARY_DIR}/gdm.tar.gz")
    set(GDM_EXTRACT_DIR "${CMAKE_BINARY_DIR}/gdm_extract")

    file(DOWNLOAD
        "${GDM_URL}"
        "${GDM_ARCHIVE}"
        TLS_VERIFY ON
        STATUS _dl_status
    )

    list(GET _dl_status 0 _dl_code)
    if(NOT _dl_code EQUAL 0)
        message(FATAL_ERROR "Failed to download gdm")
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

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generatedHeader")
configure_file(
    "${CMAKE_SOURCE_DIR}/src/aimlab/config.hpp.in"
    "${CMAKE_BINARY_DIR}/generatedHeader/config.hpp"
    @ONLY
)


# Resource Downloader
function(fetch_example_asset ASSET_NAME)
    set(_asset_dir "${PROJECT_SOURCE_DIR}/assets/${ASSET_NAME}")

    # Already available
    if (EXISTS "${_asset_dir}")
        return()
    endif()

    message(STATUS "[${PROJECT_NAME}] Fetching asset: ${ASSET_NAME}")

    # Download cache
    set(_download_dir "${CMAKE_BINARY_DIR}/_downloads")
    file(MAKE_DIRECTORY "${_download_dir}")

    set(_archive "${_download_dir}/${ASSET_NAME}.zip")

    # Download
    if (NOT EXISTS "${_archive}")
        set(_url
            "https://github.com/awidesky/glutil-examples/releases/download/0.0.1/${ASSET_NAME}.zip"
        )

        file(DOWNLOAD
            "${_url}"
            "${_archive}"
            STATUS _dl_status
            TLS_VERIFY ON
        )
        list(GET _dl_status 0 _dl_code)

        if (NOT _dl_code EQUAL 0)
            file(REMOVE "${_archive}")
        
            list(GET _dl_status 1 _dl_msg)
            message(FATAL_ERROR
                "[${PROJECT_NAME}] Failed to download asset '${ASSET_NAME}'\n"
                "URL: ${_url}\n"
                "Reason: ${_dl_msg}"
            )
        endif()
    else()
        message(STATUS "[${PROJECT_NAME}] Using cached archive: ${_archive}")
    endif()

    # Extract
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/assets")
    set(_extract_dir "${CMAKE_BINARY_DIR}/_extract/${ASSET_NAME}")

    file(REMOVE_RECURSE "${_extract_dir}")
    file(MAKE_DIRECTORY "${_extract_dir}")

    file(ARCHIVE_EXTRACT INPUT "${_archive}" DESTINATION "${_extract_dir}")
    
    # Move extracted content
    file(GLOB _children
        LIST_DIRECTORIES TRUE
        "${_extract_dir}/*"
    )

    set(_root "")

    foreach(p IN LISTS _children)
        if(IS_DIRECTORY "${p}")
            set(_root "${p}")
            break()
        endif()
    endforeach()

    # root folder exist
    if (_root)
        file(RENAME "${_root}" "${_asset_dir}")
    else()
        # no root folder
        file(MAKE_DIRECTORY "${_asset_dir}")
        file(GLOB _files "${_extract_dir}/*")

        foreach(f IN LISTS _files)
            get_filename_component(_name "${f}" NAME)
            file(RENAME "${f}" "${_asset_dir}/${_name}")
        endforeach()
    endif()
    file(REMOVE_RECURSE "${_extract_dir}")

    if (NOT EXISTS "${_asset_dir}")
        message(FATAL_ERROR "[${PROJECT_NAME}] Failed to extract asset '${ASSET_NAME}'")
    endif()
endfunction()


# Example targets
function(add_gdm_example TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_include_directories(${TARGET_NAME} PRIVATE
        "${CMAKE_BINARY_DIR}/generatedHeader"
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
    src/manyCubes.cpp

    shader/manyCubes.vert
    shader/manyCubes.frag
)
add_gdm_example(02_Excavator_bad
    src/excavator_bad.cpp

    shader/excvator.vert
    shader/excvator.frag
)
add_gdm_example(03_Excavator_good
    src/excavator_good.cpp

    shader/excvator.vert
    shader/excvator.frag
)
add_gdm_example(04_AIMLAB
    src/aimlab/AIMLAB_main.cpp
    src/aimlab/gameloop.hpp
    src/aimlab/camera.hpp
    src/aimlab/engine.hpp
    src/aimlab/component.hpp
    src/aimlab/physics.hpp
    src/aimlab/mesh.hpp
    src/aimlab/manager.hpp
	
    shader/aimlab.vert
    shader/aimlab.frag
)

fetch_example_asset(ak47)

source_group("Shaders"
    REGULAR_EXPRESSION ".*\\.(frag|vert)$"
)
