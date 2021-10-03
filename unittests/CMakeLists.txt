
# InetAddress
add_executable(testInetAddress testMain.cc InetAddressUnittest.cc ../src/net/InetAddress.cc)

# MsgBuffer
add_executable(testMsgBuffer testMain.cc MsgBufferUnittest.cc ../src/net/MsgBuffer.cc)

# Regex parse http request
add_executable(testRegex RegexUnittest.cc testMain.cc)

# Regex parse http request
add_executable(testHttpContext testMain.cc HttpContextUnittest.cc)
target_link_libraries(testHttpContext server_core pthread)

# TimerQueue test
add_executable(testTimerQueue TimerQueueUnittest.cc)
target_link_libraries(testTimerQueue server_core pthread)
