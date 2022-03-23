
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist/examples")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist/examples")

add_executable(EchoServer EchoServer.cc)
target_link_libraries(EchoServer ${JERRY_LIB} pthread)

add_executable(TimerServer TimerServer.cc)
target_link_libraries(TimerServer ${JERRY_LIB} pthread)

add_executable(DemoHttpServer DemoHttpServer.cc)
target_link_libraries(DemoHttpServer ${JERRY_LIB} pthread)

add_executable(DemoWebsocketServer DemoWebsocketServer.cc)
target_link_libraries(DemoWebsocketServer ${JERRY_LIB} pthread)