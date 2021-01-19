cmake_minimum_required(VERSION 3.16)
project(MST_openCV)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_subdirectory(threadpool/ThreadPool)

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

set(CMAKE_CXX_CPPCHECK "cppcheck")

find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)

if (CMAKE_CXX_CPPCHECK)
    list(
        APPEND CMAKE_CXX_CPPCHECK 
            "--enable=warning"
            "--inconclusive"
            "--force" 
            "--inline-suppr"
    )
endif()

add_executable(MST_openCV ${SOURCES} ${HEADERS})

target_link_libraries( MST_openCV ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} )
