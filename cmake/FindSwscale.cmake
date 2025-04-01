include(FindPackageHandleStandardArgs)

find_path(Swscale_INCLUDE_DIRS
    NAMES libswscale/swscale.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(Swscale_LIBRARIES
    NAMES swscale
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(Swscale_VERSION 1.0)

message(STATUS "swscale include path:${Swscale_INCLUDE_DIRS}")
message(STATUS "swscale library:${Swscale_LIBRARIES}")

set(Swscale_LIBRARY_DIRS "")
find_package_handle_standard_args(Swscale
        FOUND_VAR Swscale_FOUND
        REQUIRED_VARS Swscale_INCLUDE_DIRS
                      Swscale_LIBRARIES
                      Swscale_VERSION
        )

if(Swscale_FOUND AND NOT TARGET Swscale)
    foreach(FILE ${Swscale_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND Swscale_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(Swscale UNKNOWN IMPORTED)
    set_target_properties(Swscale PROPERTIES
                IMPORTED_LOCATION "${Swscale_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Swscale_INCLUDE_DIRS}")
endif()
message(STATUS "swscale library dirs:${Swscale_LIBRARY_DIRS}")

mark_as_advanced(
    Swscale_INCLUDE_DIRS
    Swscale_LIBRARIES
    Swscale_VERSION
    Swscale_LIBRARY_DIRS
)