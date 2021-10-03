add_executable(myServlet main.cc)
target_link_libraries(myServlet server_core pthread)