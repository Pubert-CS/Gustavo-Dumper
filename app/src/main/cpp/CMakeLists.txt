cmake_minimum_required(VERSION 3.22.1)
project("androidscaffolder")

add_compile_definitions(kNO_KEYSTONE)
file(GLOB_RECURSE KITTY_MEMORY_SOURCES extern/KittyMemory/KittyMemory/*.cpp)

add_library(${CMAKE_PROJECT_NAME} SHARED
        native-lib.cpp
        il2cpp/mono/MonoString.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    .
        extern/KittyMemory/KittyMemory
        extern/json/single_include
        extern/StringUtils
)

target_link_libraries(${CMAKE_PROJECT_NAME}
        android
        log)