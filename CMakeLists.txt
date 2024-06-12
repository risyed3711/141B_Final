cmake_minimum_required(VERSION 3.10)
project(ECE141db)


set(CMAKE_CXX_STANDARD 17)


include_directories(include)


file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

set(SOURCES ${SOURCES} ${HEADERS}
        src/misc/Join.cpp
        src/misc/Logger.cpp
)



add_executable(ECE141db ${SOURCES})
