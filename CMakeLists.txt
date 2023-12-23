cmake_minimum_required(VERSION 3.9)
project(MultiRename)

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 17)

add_executable(main
    main.cpp
)

set(LIBS
    Rpcrt4
)

target_link_libraries(main PRIVATE ${LIBS})
