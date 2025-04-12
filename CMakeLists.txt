cmake_minimum_required(VERSION 3.9)
project(Concurrent)
cmake_policy(SET CMP0115 OLD)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(./)

add_library(concurrent
	time.cpp
)

