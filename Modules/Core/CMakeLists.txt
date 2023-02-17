cmake_minimum_required(VERSION 3.10)

# Module Name
project(Core VERSION 1.0)
if(${DEBUG_BUILD})
message("[+]DEBUG BUILD!")
else()
message("[+]RELEASE BUILD!")
set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-o3 -DNDEBUG")

# If BUILD_<Lib>_LIB is OFF, then it's respective executable will be generated. This is to provide ease of testing until 
# unit testing is available
set(BUILD_LOGGER_LIB ON)
set(BUILD_MEMORY_LIB ON)
set(BUILD_IPC_LIB ON)
set(BUILD_TYPES_LIB ON)

if(${BUILD_CORE_LIB})
    message("[+]Building Core Library")
else()
    set(EXECUTABLE_NAME "Output")
    set(BUILD_CORE_LIB OFF)
    set(BUILD_MEMORY_LIB OFF)
    message("[+]Building Core Executables")
endif()

if(${BUILD_CORE_LIB})
    message("[+]Added target Library " ${PROJECT_NAME})
    add_library(${PROJECT_NAME})
endif()

#Include External Modules(CMakeLists.txt) here
include("${CMAKE_CURRENT_LIST_DIR}/Logger/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/Memory/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/IPC/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/Types/CMakeLists.txt")
