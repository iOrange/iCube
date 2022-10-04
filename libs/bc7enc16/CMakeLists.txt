cmake_minimum_required(VERSION 3.8)
project(bc7enc CXX C)

set(HEADERS
    bc7enc16.h
    bc7decomp.h)

set(SOURCES
    bc7enc16.c
    bc7decomp.c)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
