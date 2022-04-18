
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist/tests")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist/tests")

remove_definitions(-DUSE_OPENSSL)
add_executable(BaseBufferTest CatchMain.cc BaseBufferTest.cc ../src/net/BaseBuffer.cc)

add_executable(CountDownLatchTest CountDownLatchTest.cc ../src/utils/CountDownLatch.cc)
target_link_libraries(CountDownLatchTest pthread)
