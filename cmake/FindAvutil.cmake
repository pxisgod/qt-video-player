include(FindPackageHandleStandardArgs)

find_path(Avutil_INCLUDE_DIRS
    NAMES libavutil/avutil.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(Avutil_LIBRARIES
    NAMES avutil
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(Avutil_VERSION 1.0)

message(STATUS "Avutil include path:${Avutil_INCLUDE_DIRS}")
message(STATUS "Avutil library:${Avutil_LIBRARIES}")

set(Avutil_LIBRARY_DIRS "")
find_package_handle_standard_args(Avutil
        FOUND_VAR Avutil_FOUND
        REQUIRED_VARS Avutil_INCLUDE_DIRS
                      Avutil_LIBRARIES
                      Avutil_VERSION
        )

if(Avutil_FOUND AND NOT TARGET Avutil)
    foreach(FILE ${Avutil_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND Avutil_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(Avutil UNKNOWN IMPORTED)
    set_target_properties(Avutil PROPERTIES
                IMPORTED_LOCATION "${Avutil_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Avutil_INCLUDE_DIRS}")
endif()
message(STATUS "Avutil library dirs:${Avutil_LIBRARY_DIRS}")

mark_as_advanced(
    Avutil_INCLUDE_DIRS
    Avutil_LIBRARIES
    Avutil_VERSION
    Avutil_LIBRARY_DIRS
)