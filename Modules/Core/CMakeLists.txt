cmake_minimum_required(VERSION 3.10)

project(Core VERSION 1.0)
# Module Name

if(NOT DEFINED ${BUILD_CORE_LIB})
if(${DEBUG_BUILD})
    message("[+]DEBUG BUILD!")
    set(CMAKE_BUILD_TYPE Debug)
    add_compile_definitions(DEBUG_BUILD)
else()
    message("[+]RELEASE BUILD!")
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()
endif()

if(MSVC)
    add_compile_definitions(INTRINSICS_SUPPORTED)
    message("[+]Platform Intrics support macro added. Remove this flag if intrinsics are not supported/required.")
    if("${BUILD_PLATFORM}" STREQUAL "WIN32")
        message("[+]Added SSE2 intrinsics")
        add_compile_options(/arch:SSE2)
    else()
        message("[+]Added AVX2 intrinsics")
        add_compile_options(/arch:AVX2)
    endif()
endif()

# If BUILD_<Lib>_LIB is OFF, then it's respective executable will be generated. This is to provide ease of testing until 
# unit testing is available
set(BUILD_LOGGER_LIB ON)
set(BUILD_MEMORY_LIB ON)
set(BUILD_IPC_LIB ON)
set(BUILD_TYPES_LIB ON)
set(BUILD_MATH_LIB ON)

if(${BUILD_CORE_LIB})
    message("[+]Building Core Library")
else()
    add_compile_definitions(QUAINT_PLATFORM_${BUILD_PLATFORM})
    
    set(EXECUTABLE_NAME "Output")
    set(BUILD_CORE_LIB OFF)
    set(BUILD_MATH_LIB OFF)
    message("[+]Building Core Executables")
endif()

if(${BUILD_CORE_LIB})
    project(Core VERSION 1.0)
    message("[+]Added target Library " ${PROJECT_NAME})
    add_library(${PROJECT_NAME})
endif()

message("[+]Core Binary Dir: " ${CMAKE_BINARY_DIR} " Build Type: "  ${CMAKE_BUILD_TYPE})
#Include External Modules(CMakeLists.txt) here
#add_subdirectory(Logger ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
#add_subdirectory(Memory ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
#add_subdirectory(IPC ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
#add_subdirectory(Types ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})

include("${CMAKE_CURRENT_LIST_DIR}/Logger/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/Memory/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/Math/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/IPC/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/Types/CMakeLists.txt")


if(NOT ${BUILD_CORE_LIB})
    project(Core VERSION 1.0)
endif()
