cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_CRYPTO")#我们提示CMake用c的方式进行编译,否则会默认使用c++的方式进行编译。
add_library(
        rtmp#生成一个名为rtmp的静态库
        STATIC  #静态库
        amf.c   #下面是需要的.c文件，可以在rtmp饿makefile中看到
        rtmp.c
        hashswf.c
        log.c
        parseurl.c
        )
