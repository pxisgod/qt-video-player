qt5_add_resources(RESOURCES resources.qrc)
file(GLOB SRC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
list(APPEND SRC_FILE ${RESOURCES})
set(TARGET_NAME "video_player")
add_executable(${TARGET_NAME} ${SRC_FILE})
target_include_directories(${TARGET_NAME} 
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_link_libraries(${TARGET_NAME} 
            PUBLIC
                $<BUILD_INTERFACE:${LINK_LIBRARIES_DEFAULT}>
                $<INSTALL_INTERFACE:${LINK_LIBRARIES_DEFAULT}>)
set_property(TARGET ${TARGET_NAME} APPEND PROPERTY AUTOUIC ON)
set_property(TARGET ${TARGET_NAME} APPEND PROPERTY AUTORCC ON)
set_property(TARGET ${TARGET_NAME} APPEND PROPERTY AUTOMOC ON)