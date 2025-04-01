include(FindPackageHandleStandardArgs)

find_path(SDL2_INCLUDE_DIRS
    NAMES SDL2/SDL.h)

find_library(SDL2_LIBRARIES
    NAMES SDL2)

set(SDL2_VERSION 1.0)

message(STATUS "SDL2 include path:${SDL2_INCLUDE_DIRS}")
message(STATUS "SDL2 library:${SDL2_LIBRARIES}")

set(SDL2_LIBRARY_DIRS "")
find_package_handle_standard_args(SDL2
        FOUND_VAR SDL2_FOUND
        REQUIRED_VARS SDL2_INCLUDE_DIRS
                      SDL2_LIBRARIES
                      SDL2_VERSION
        )

if(SDL2_FOUND AND NOT TARGET SDL2)
    foreach(FILE ${SDL2_LIBRARIES})
        get_filename_component(LIB_DIR ${FILE} PATH)
        list(APPEND SDL2_LIBRARY_DIRS "${LIB_DIR}")
    endforeach()
    add_library(SDL2 UNKNOWN IMPORTED)
    set_target_properties(SDL2 PROPERTIES
                IMPORTED_LOCATION "${SDL2_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${SDL2_INCLUDE_DIRS}")
endif()
message(STATUS "SDL2 library dirs:${SDL2_LIBRARY_DIRS}")

mark_as_advanced(
    SDL2_INCLUDE_DIRS
    SDL2_LIBRARIES
    SDL2_VERSION
    SDL2_LIBRARY_DIRS
)