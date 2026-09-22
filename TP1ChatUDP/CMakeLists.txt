
# Add client and server executables
add_executable (server "server.cpp" "server.h")
add_executable (client "client.cpp" "client.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET server PROPERTY CXX_STANDARD 20)
  set_property(TARGET client PROPERTY CXX_STANDARD 20)
endif()

