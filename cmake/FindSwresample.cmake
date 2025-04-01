include(FindPackageHandleStandardArgs)

find_path(Swresample_INCLUDE_DIRS
    NAMES libswresample/swresample.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(Swresample_LIBRARIES
    NAMES swresample
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(Swresample_VERSION 1.0)

message(STATUS "swresample include path:${Swresample_INCLUDE_DIRS}")
message(STATUS "swresample library:${Swresample_LIBRARIES}")

set(Swresample_LIBRARY_DIRS "")
find_package_handle_standard_args(Swresample
        FOUND_VAR Swresample_FOUND
        REQUIRED_VARS Swresample_INCLUDE_DIRS
                      Swresample_LIBRARIES
                      Swresample_VERSION
        )

if(Swresample_FOUND AND NOT TARGET Swresample)
    foreach(FILE ${Swresample_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND Swresample_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(Swresample UNKNOWN IMPORTED)
    set_target_properties(Swresample PROPERTIES
                IMPORTED_LOCATION "${Swresample_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Swresample_INCLUDE_DIRS}")
endif()
message(STATUS "swresample library dirs:${Swresample_LIBRARY_DIRS}")

mark_as_advanced(
    Swresample_INCLUDE_DIRS
    Swresample_LIBRARIES
    Swresample_VERSION
    Swresample_LIBRARY_DIRS
)