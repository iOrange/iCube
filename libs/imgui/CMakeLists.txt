cmake_minimum_required(VERSION 3.8)
project(dear_imgui CXX)

set(HEADERS
    imconfig.h
    imgui.h
)

set(SOURCES
    imgui.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
