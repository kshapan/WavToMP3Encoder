cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 17)

project(MP3Encoder)

include_directories(lame/include/)

set(SRC src/main.cpp)

add_executable(${PROJECT_NAME} ${SRC})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libmp3lame-static.lib)
else()
    target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libmp3lame.a pthread)
endif()