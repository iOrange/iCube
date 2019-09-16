project(glad C)

file(GLOB_RECURSE HEADERS "include/*.h")
file(GLOB_RECURSE SOURCES "src/*.c")

include_directories("include")

add_library(${PROJECT_NAME} ${HEADERS} ${SOURCES})
