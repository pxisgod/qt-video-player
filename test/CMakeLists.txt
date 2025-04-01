
file(GLOB CPPS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
foreach(CPP ${CPPS})
	# 测试目录下的每一个文件就是一个测试
    get_filename_component(TARGET_NAME ${CPP} NAME)
    string(REPLACE ".cpp" "" TARGET_NAME ${TARGET_NAME})
    add_executable(${TARGET_NAME} ${CPP})
    add_test(${TARGET_NAME} ${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} 
            PUBLIC
                $<BUILD_INTERFACE:${LINK_LIBRARIES_DEFAULT} Qt5::Test>
                $<INSTALL_INTERFACE:${LINK_LIBRARIES_DEFAULT} Qt5::Test>)
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY AUTOUIC ON)
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY AUTORCC ON)
    set_property(TARGET ${TARGET_NAME} APPEND PROPERTY AUTOMOC ON)
endforeach()