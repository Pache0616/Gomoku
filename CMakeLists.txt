cmake_minimum_required(VERSION 3.10)
project(Gomoku CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


set(SOURCES
    src/main.cpp
    src/ai.cpp
    src/logic.cpp
    src/ui.cpp
)


add_executable(Gomoku WIN32 ${SOURCES})

target_include_directories(Gomoku PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/easyx/include
)

target_link_directories(Gomoku PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/easyx/lib/x64
)

target_link_libraries(Gomoku PRIVATE 
    easyx
    gdi32
    ole32
)