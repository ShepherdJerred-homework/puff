cmake_minimum_required(VERSION 3.8)
project(puff)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        puff/puff.cpp
        puff/stdafx.cpp
        puff/stdafx.h
        puff/targetver.h)

add_executable(puff ${SOURCE_FILES})