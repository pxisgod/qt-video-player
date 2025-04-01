cmake_minimum_required(VERSION 3.10)

project(QtVideoPlayer)

set(CMAKE_CXX_STANDARD 17)

set( CMAKE_PREFIX_PATH "/home/px/Qt5.12.12/5.12.12/gcc_64/lib/cmake")

function(update_cache name)
   set("${name}"  "${ARGN}" CACHE INTERNAL "" FORCE) #设置变量为全局变量，就不用考虑作用域的问题了
endfunction()

function(append_cache name)
   set(_item_list "${${name}}")
   list(APPEND _item_list "${ARGN}") #添加属性
   list(REMOVE_DUPLICATES _item_list )
   update_cache("${name}" "${_item_list}")
endfunction()


# 查找Qt库
find_package(Qt5 5.12 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets)
append_cache(LINK_LIBRARIES_DEFAULT Qt5::Core)
append_cache(LINK_LIBRARIES_DEFAULT Qt5::Widgets)
append_cache(LINK_LIBRARIES_DEFAULT Qt5::Multimedia)
append_cache(LINK_LIBRARIES_DEFAULT Qt5::MultimediaWidgets)

set(FFMPEG_DIR "/usr/local/ffmpeg")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(Avformat QUIET)
if(Avformat_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT Avformat)
endif()
find_package(Avcodec QUIET)
if(Avcodec_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT Avcodec)
endif()
find_package(Avfilter QUIET)
if(Avfilter_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT Avfilter)
endif()
find_package(Swresample QUIET)
if(Swresample_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT Swresample)
endif()
find_package(Swscale QUIET)
if(Swscale_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT Swscale)
endif()
find_package(Avutil QUIET)
if(Avutil_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT Avutil)
endif()
find_package(X264 QUIET)
if(X264_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT X264)
endif()
find_package(SDL2 QUIET)
if(SDL2_FOUND)
   append_cache(LINK_LIBRARIES_DEFAULT SDL2)
endif()




add_subdirectory(player)
add_subdirectory(src)
find_package(Qt5 5.12 REQUIRED COMPONENTS Test)
if(Qt5Test_FOUND)
    enable_testing()
    add_subdirectory(test)
endif()


