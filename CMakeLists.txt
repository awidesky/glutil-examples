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

add_subdirectory("${GDM_DIR}")


# Example targets
function(add_gdm_example TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
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
    src/aimlab/camera.hpp
    src/aimlab/engine.hpp

    shader/aimlab.vert
    shader/aimlab.frag
)

source_group("Shaders"
    REGULAR_EXPRESSION ".*\\.(frag|vert)$"
)
