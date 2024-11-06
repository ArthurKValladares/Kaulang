cmake_minimum_required(VERSION 3.25)
project(kaulang)

add_definitions(-DDEBUG=1) 

add_executable(${PROJECT_NAME}
    src/main.cpp
)