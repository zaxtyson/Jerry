add_executable(myServlet main.cc)
target_link_libraries(myServlet server_core pthread)

# copy webroot directory to BIN path
message(STATUS "Copy webroot to ${CMAKE_CURRENT_BINARY_DIR}")
file(COPY webroot DESTINATION ${CMAKE_CURRENT_BINARY_DIR})