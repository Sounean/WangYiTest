cmake_minimum_required(VERSION 3.4.1)


add_subdirectory(librtmp)#引入rtmpDump
add_library(
        native-lib
        SHARED
        native-lib.cpp
        VideoChannel.cpp
        AudioChannel.cpp)

include_directories(include)#把include目录下的x264.h,x264_config.h头文件给引进来。
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/libs/${ANDROID_ABI}")#-L代表路径的意思。-L${CMAKE_SOURCE_DIR}代表着CMakeLists.txt的路径。
find_library(
        log-lib
        log)

target_link_libraries(
        native-lib
        rtmp#引入在librtmp文件夹下新建的CMakeList里新建的名为rtmp的静态库
        ${log-lib}
        x264#寻找名为x264的静态库
        faac)