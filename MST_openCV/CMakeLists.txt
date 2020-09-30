cmake_minimum_required(VERSION 3.16)
project(MST_openCV)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(HEADERS
        headers/GraphProcessor.h
        )

set(SOURCES
        main.cpp
        src/GraphProcessor.cpp
        )

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/headers
)

add_executable(MST_openCV ${SOURCES} ${HEADERS})

target_link_libraries( MST_openCV ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} )