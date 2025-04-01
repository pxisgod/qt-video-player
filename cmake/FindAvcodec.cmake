include(FindPackageHandleStandardArgs)

find_path(Avcodec_INCLUDE_DIRS
    NAMES libavcodec/avcodec.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(Avcodec_LIBRARIES
    NAMES avcodec
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(Avcodec_VERSION 1.0)

message(STATUS "avcodec include path:${Avcodec_INCLUDE_DIRS}")
message(STATUS "avcodec library:${Avcodec_LIBRARIES}")

set(Avcodec_LIBRARY_DIRS "")
find_package_handle_standard_args(Avcodec
        FOUND_VAR Avcodec_FOUND
        REQUIRED_VARS Avcodec_INCLUDE_DIRS
                      Avcodec_LIBRARIES
                      Avcodec_VERSION
        )

if(Avcodec_FOUND AND NOT TARGET Avcodec)
    foreach(FILE ${Avcodec_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND Avcodec_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(Avcodec UNKNOWN IMPORTED)
    set_target_properties(Avcodec PROPERTIES
                IMPORTED_LOCATION "${Avcodec_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Avcodec_INCLUDE_DIRS}")
endif()
message(STATUS "avcodec library dirs:${Avcodec_LIBRARY_DIRS}")

mark_as_advanced(
    Avcodec_INCLUDE_DIRS
    Avcodec_LIBRARIES
    Avcodec_VERSION
    Avcodec_LIBRARY_DIRS
)