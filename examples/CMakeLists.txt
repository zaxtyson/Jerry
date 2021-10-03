
add_executable(EchoServer EchoServer.cc)
target_link_libraries(EchoServer server_core pthread)


add_executable(StaticPage StaticPage.cc)
target_link_libraries(StaticPage server_core pthread)

add_subdirectory(servlet)