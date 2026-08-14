cmake_minimum_required(VERSION 3.15)
project(chat_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# FetchContent is used to download and build Asio in order to avoid installign it on the system
include(FetchContent)
FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-36-0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(asio)

add_executable(chat_server
  src/main.cpp
  src/ChatServer.cpp
  src/ChatRoom.cpp
  src/ChatSession.cpp
)

target_include_directories(chat_server PRIVATE
  include
  ${asio_SOURCE_DIR}/asio/include
)

if (WIN32)
  target_compile_definitions(chat_server PRIVATE _WIN32_WINNT=0x0601)
endif()


