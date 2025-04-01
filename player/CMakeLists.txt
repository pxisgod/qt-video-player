file(GLOB SRC_FILE
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/util/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/decoder/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/demuxer/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/render/video/*.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/render/audio/*.cpp)
set(TARGET_NAME "player_lib")
add_library(${TARGET_NAME} SHARED ${SRC_FILE})
target_include_directories(${TARGET_NAME} 
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}
                                    ${CMAKE_CURRENT_SOURCE_DIR}/decoder
                                    ${CMAKE_CURRENT_SOURCE_DIR}/demuxer
                                    ${CMAKE_CURRENT_SOURCE_DIR}/render/audio 
                                    ${CMAKE_CURRENT_SOURCE_DIR}/render/video   >
                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${TARGET_NAME} 
            PUBLIC
                $<BUILD_INTERFACE:${LINK_LIBRARIES_DEFAULT}>
                $<INSTALL_INTERFACE:${LINK_LIBRARIES_DEFAULT}>)

append_cache(LINK_LIBRARIES_DEFAULT ${TARGET_NAME})