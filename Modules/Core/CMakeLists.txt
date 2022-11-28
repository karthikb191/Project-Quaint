cmake_minimum_required(VERSION 3.10)

# Module Name
project(Core VERSION 1.0)

if(${BUILD_BOLT_LIB})
    message("[+]Building Core Library")
else()
    message("[+]Building Core Executable")
endif()

#Include External Modules(CMakeLists.txt) here
include("${CMAKE_CURRENT_LIST_DIR}/Logger/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/Memory/CMakeLists.txt")

#TODO: Register sub modules here
#add_subdirectory(Renderer)   # look in hello subdirectory for CMakeLists.txt to process
#add_subdirectory(Logger)    # look in apps subdirectory for CMakeLists.txt to process