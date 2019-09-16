project(bc7enc C)

cmake_minimum_required(VERSION 2.8)

set(HEADERS
    bc7enc16.h
    bc7decomp.h)

set(SOURCES
    bc7enc16.c
    bc7decomp.c)

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
