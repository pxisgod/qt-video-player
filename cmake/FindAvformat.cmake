include(FindPackageHandleStandardArgs)

find_path(Avformat_INCLUDE_DIRS
    NAMES libavformat/avformat.h
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES include)

find_library(Avformat_LIBRARIES
    NAMES avformat
    HINTS ${FFMPEG_DIR}
    PATH_SUFFIXES lib)

set(Avformat_VERSION 1.0)

message(STATUS "avformat include path:${Avformat_INCLUDE_DIRS}")
message(STATUS "avformat library:${Avformat_LIBRARIES}")

set(Avformat_LIBRARY_DIRS "")
find_package_handle_standard_args(Avformat
        FOUND_VAR Avformat_FOUND
        REQUIRED_VARS Avformat_INCLUDE_DIRS
                      Avformat_LIBRARIES
                      Avformat_VERSION
        )

if(Avformat_FOUND AND NOT TARGET Avformat)
    foreach(FILE ${Avformat_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND Avformat_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(Avformat UNKNOWN IMPORTED)
    set_target_properties(Avformat PROPERTIES
                IMPORTED_LOCATION "${Avformat_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${Avformat_INCLUDE_DIRS}")
endif()
message(STATUS "avformat library dirs:${Avformat_LIBRARY_DIRS}")

mark_as_advanced(
    Avformat_INCLUDE_DIRS
    Avformat_LIBRARIES
    Avformat_VERSION
    Avformat_LIBRARY_DIRS
)