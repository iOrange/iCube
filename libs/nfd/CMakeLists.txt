cmake_minimum_required(VERSION 3.1)

#NOTE_SK: this is my attempt to port SConstruct script to CMake

project(nfd CXX)

set(SOURCES "src/nfd_common.c")

if(UNIX AND NOT APPLE)
    set(SOURCES ${SOURCES} "src/nfd_gtk.c")
endif()

if(APPLE)
    set(SOURCES ${SOURCES} "src/nfd_cocoa.m")
endif()

if(WIN32)
    set(SOURCES ${SOURCES} "src/nfd_win.cpp")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include_directories("src/include")

add_library(${PROJECT_NAME} ${SOURCES})
