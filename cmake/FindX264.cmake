include(FindPackageHandleStandardArgs)

find_path(X264_INCLUDE_DIRS
    NAMES libx264/x264.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(X264_LIBRARIES
    NAMES x264
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(X264_VERSION 1.0)

message(STATUS "x264 include path:${X264_INCLUDE_DIRS}")
message(STATUS "x264 library:${X264_LIBRARIES}")

set(X264_LIBRARY_DIRS "")
find_package_handle_standard_args(X264
        FOUND_VAR X264_FOUND
        REQUIRED_VARS X264_INCLUDE_DIRS
                      X264_LIBRARIES
                      X264_VERSION
        )

if(X264_FOUND AND NOT TARGET X264)
    foreach(FILE ${X264_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND X264_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(X264 UNKNOWN IMPORTED)
    set_target_properties(X264 PROPERTIES
                IMPORTED_LOCATION "${X264_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${X264_INCLUDE_DIRS}")
endif()
message(STATUS "x264 library dirs:${X264_LIBRARY_DIRS}")

mark_as_advanced(
    X264_INCLUDE_DIRS
    X264_LIBRARIES
    X264_VERSION
    X264_LIBRARY_DIRS
)