cmake_minimum_required(VERSION 3.20)
project(Jerry)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")

include_directories(${PROJECT_SOURCE_DIR}/src)

################################

# 是否编译单元测试
set(UNITTEST ON)

# 是否编译测试程序
set(BUILD_EXAMPLES ON)

# 是否打印日志
set(SHOW_LOG ON)

# 日志显示具体文件,行号,函数名
set(SHOW_LOG_DETAIL ON)

################################

if (UNITTEST)
    MESSAGE("Start unit tests...")
    add_subdirectory(unittests)
endif ()

if(SHOW_LOG)
    add_definitions(-D SHOW_LOG)
endif ()

if(SHOW_LOG_DETAIL)
    add_definitions(-DSHOW_LOG_DETAIL)
endif()

if (BUILD_EXAMPLES)
    aux_source_directory(src/utils SRC_LIST)
    aux_source_directory(src/net SRC_LIST)
    aux_source_directory(src/http SRC_LIST)
    add_library(server_core SHARED ${SRC_LIST})  # 编译共享库, 方便 examples 使用
    add_subdirectory(examples)
endif ()