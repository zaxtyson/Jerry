
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist/examples")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/dist/examples")

file(COPY pem DESTINATION ${EXECUTABLE_OUTPUT_PATH})

add_executable(EchoServer EchoServer.cc)
target_link_libraries(EchoServer ${JERRY_LIB} pthread ssl crypto)

add_executable(TimerServer TimerServer.cc)
target_link_libraries(TimerServer ${JERRY_LIB} pthread ssl crypto)

add_executable(DemoHttpServer DemoHttpServer.cc)
target_link_libraries(DemoHttpServer ${JERRY_LIB} pthread ssl crypto)

add_executable(DemoWebsocketServer DemoWebsocketServer.cc)
target_link_libraries(DemoWebsocketServer ${JERRY_LIB} pthread ssl crypto)