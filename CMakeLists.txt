#设置cmake 最低版本
cmake_minimum_required(VERSION 3.1)

#设置系统平台
set(CMAKE_SYSTEM_NAME Linux)

# set C and CPP compile flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-pointer-arith")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# set Cmake enviroment
include(env/env.cmake)

# 增加需要编译的子文件夹
add_subdirectory(app)
add_subdirectory(thirdparty/libfaac)
add_subdirectory(thirdparty/libfaad)

