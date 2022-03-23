cmake_minimum_required(VERSION 3.0)
project(Jerry)

set(CMAKE_CXX_STANDARD 17)
set(COMMON_CXX_FLAGS "-g -Wall -Wextra -Wpedantic -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS_DEBUG "${COMMON_CXX_FLAGS} -O0 -fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${COMMON_CXX_FLAGS} -O3 -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist")

########################
#   Compile  Options   #
########################

set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIB ON)
set(BUILD_TESTS ON)
set(BUILD_EXAMPLES ON)
set(BUILD_WRK ON)
set(SHOW_LOG ON)

########################

message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

set(JERRY_LIB jerry)

include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty)
include_directories(${PROJECT_SOURCE_DIR}/3rdparty/Catch2/single_include)

if (SHOW_LOG)
    message(STATUS "Enable logger for project")
    add_definitions(-DSHOW_LOG)
endif ()

if (BUILD_SHARED_LIB)
    message(STATUS "Enable build shared library: lib${JERRY_LIB}.so")
    aux_source_directory(src/utils SRC_LIST)
    aux_source_directory(src/logger SRC_LIST)
    aux_source_directory(src/net SRC_LIST)
    aux_source_directory(src/proto/http SRC_LIST)
    aux_source_directory(src/proto/ws SRC_LIST)
    aux_source_directory(3rdparty/crypto SRC_LIST)

    add_library(${JERRY_LIB} SHARED ${SRC_LIST})
    set_target_properties(
            ${JERRY_LIB} PROPERTIES
            VERSION 0.5.0
            SOVERSION 0
    )
endif ()

if (BUILD_TESTS)
    message(STATUS "Enable build tests")
    add_subdirectory(tests)
endif ()

if (BUILD_EXAMPLES)
    message(STATUS "Enable build examples")
    add_subdirectory(examples)
endif ()

if (BUILD_WRK)
    message(STATUS "Enable build wrk")
    execute_process(COMMAND make -j8 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/3rdparty/wrk)
    file(COPY 3rdparty/wrk/wrk DESTINATION ${EXECUTABLE_OUTPUT_PATH})
    message(STATUS "wrk => ${EXECUTABLE_OUTPUT_PATH}/wrk")
endif ()