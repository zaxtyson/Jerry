cmake_minimum_required(VERSION 3.17)
project(Jerry)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Release")

include_directories(${PROJECT_SOURCE_DIR}/src)

################################

# 是否编译共享库
set(BUILD_SHARED_LIB ON)

# 是否编译单元测试
set(BUILD_UNITTEST ON)

# 是否编译测试程序
set(BUILD_EXAMPLES ON)

# 是否打印日志
set(SHOW_LOG ON)

################################

if (SHOW_LOG)
    message(STATUS "Enable logger for project")
    add_definitions(-DSHOW_LOG)
endif ()

if (BUILD_SHARED_LIB)
    message(STATUS "Enable build shared libraries: libserver_core.so")
    aux_source_directory(src/utils SRC_LIST)
    aux_source_directory(src/utils/log SRC_LIST)
    aux_source_directory(src/net SRC_LIST)
    aux_source_directory(src/http SRC_LIST)
    add_library(server_core SHARED ${SRC_LIST})  # 编译共享库, 方便 examples 使用
endif ()

if (BUILD_UNITTEST)
    message(STATUS "Enable build unittests")
    add_subdirectory(unittests)
endif ()

if (BUILD_EXAMPLES)
    message(STATUS "Enable build examples")
    add_subdirectory(examples)
endif ()