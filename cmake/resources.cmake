
# Resource Downloader
function(fetch_example_asset ASSET_NAME)
    set(_asset_dir "${PROJECT_SOURCE_DIR}/assets/${ASSET_NAME}")

    # Already available
    if (EXISTS "${_asset_dir}")
        return()
    endif()

    message(STATUS "[${PROJECT_NAME}] Fetching asset: ${ASSET_NAME}")

    # Temporary archive path
    set(_archive "${CMAKE_BINARY_DIR}/${ASSET_NAME}.zip")

    # Download URL
    set(_url
        "https://github.com/awidesky/glutil-examples/releases/download/0.0.1/${ASSET_NAME}.zip"
    )

    # Ensure no stale file exists
    file(REMOVE "${_archive}")

    # Download
    file(DOWNLOAD
        "${_url}"
        "${_archive}"
        STATUS _dl_status
        LOG _dl_log
    )

    list(GET _dl_status 0 _dl_code)

    if (NOT _dl_code EQUAL 0)
        file(REMOVE "${_archive}")

        list(GET _dl_status 1 _dl_msg)
        message(FATAL_ERROR
            "[${PROJECT_NAME}] Failed to download asset '${ASSET_NAME}'\n"
            "URL: ${_url}\n"
            "Reason: ${_dl_msg}"
            "Log : ${_dl_log}"
        )
    endif()

    # Extract
    file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/assets")
    set(_extract_dir "${CMAKE_BINARY_DIR}/_extract/${ASSET_NAME}")

    file(REMOVE_RECURSE "${_extract_dir}")
    file(MAKE_DIRECTORY "${_extract_dir}")

    file(ARCHIVE_EXTRACT INPUT "${_archive}" DESTINATION "${_extract_dir}")

    # Remove temporary archive immediately after extraction
    file(REMOVE "${_archive}")

    # Move extracted content
    file(GLOB _children
        LIST_DIRECTORIES TRUE
        "${_extract_dir}/*"
    )

    # Make asset dir
    file(MAKE_DIRECTORY "${_asset_dir}")

    foreach(item IN LISTS _children)
        get_filename_component(name "${item}" NAME)
        file(RENAME "${item}" "${_asset_dir}/${name}")
    endforeach()

    # Cleanup extraction directory
    file(REMOVE_RECURSE "${_extract_dir}")

    if (NOT EXISTS "${_asset_dir}")
        message(FATAL_ERROR "[${PROJECT_NAME}] Failed to extract asset '${ASSET_NAME}'")
    endif()
endfunction()


# download a file from github repository; to download single-header library
function(download_repo_file)
    set(options)
    set(oneValueArgs
        REPOSITORY
        TAG
        FILE
        DESTINATION_DIR
    )

    cmake_parse_arguments(DLF
        "${options}"
        "${oneValueArgs}"
        ""
        ${ARGN}
    )

    if (NOT DLF_REPOSITORY OR NOT DLF_TAG OR NOT DLF_FILE OR NOT DLF_DESTINATION_DIR)
        message(FATAL_ERROR "download_repo_file requires " "REPOSITORY, TAG, FILE, DESTINATION_DIR")
    endif()

    get_filename_component(
        _filename
        "${DLF_FILE}"
        NAME
    )
    set(_dst "${DLF_DESTINATION_DIR}/${_filename}")

    # already exists
    if (EXISTS "${_dst}")
        return()
    endif()

    file(MAKE_DIRECTORY "${DLF_DESTINATION_DIR}")

    string(
        REGEX REPLACE
        "^https://github.com/"
        "https://raw.githubusercontent.com/"
        _raw_repo
        "${DLF_REPOSITORY}"
    )

    set(_url "${_raw_repo}/${DLF_TAG}/${DLF_FILE}")
    message(STATUS "[${PROJECT_NAME}] Downloading ${DLF_FILE}")

    file(DOWNLOAD
        "${_url}"
        "${_dst}"
        STATUS _dl_status
    )

    list(GET _dl_status 0 _dl_code)

    if (NOT _dl_code EQUAL 0)
        file(REMOVE "${_dst}")
        list(GET _dl_status 1 _dl_msg)
        message(FATAL_ERROR
            "[${PROJECT_NAME}] Failed to download file\n"
            "URL: ${_url}\n"
            "Reason: ${_dl_msg}"
        )
    endif()
endfunction()
