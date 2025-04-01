include(FindPackageHandleStandardArgs)

find_path(Avfilter_INCLUDE_DIRS
    NAMES libavfilter/avfilter.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(Avfilter_LIBRARIES
    NAMES avfilter
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(Avfilter_VERSION 1.0)

message(STATUS "avfilter include path:${Avfilter_INCLUDE_DIRS}")
message(STATUS "avfilter library:${Avfilter_LIBRARIES}")

set(Avfilter_LIBRARY_DIRS "")
find_package_handle_standard_args(Avfilter
        FOUND_VAR Avfilter_FOUND
        REQUIRED_VARS Avfilter_INCLUDE_DIRS
                      Avfilter_LIBRARIES
                      Avfilter_VERSION
        )

if(Avfilter_FOUND AND NOT TARGET Avfilter)
    foreach(FILE ${Avfilter_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND Avfilter_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(Avfilter UNKNOWN IMPORTED)
    set_target_properties(Avfilter PROPERTIES
                IMPORTED_LOCATION "${Avfilter_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Avfilter_INCLUDE_DIRS}")
endif()
message(STATUS "avfilter library dirs:${Avfilter_LIBRARY_DIRS}")

mark_as_advanced(
    Avfilter_INCLUDE_DIRS
    Avfilter_LIBRARIES
    Avfilter_VERSION
    Avfilter_LIBRARY_DIRS
)