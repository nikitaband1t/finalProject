project(boostFinal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -lboost_system -lboost_thread")

set(SOURCE_FILES main.cpp)
add_executable(final ${SOURCE_FILES})