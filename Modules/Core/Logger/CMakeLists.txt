cmake_minimum_required(VERSION 3.10)

# Module Name
project(QuaintLogger VERSION 1.0)

# Compile features

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(${BUILD_BOLT_LIB})
    message("[+]Building Quaint-Logger Library")
    add_library(${PROJECT_NAME} ${SOURCES})
else()
    message("[+]Building Quaint-Logger Executable")
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

#TODO: Register sub modules here
#add_subdirectory(Renderer)   # look in hello subdirectory for CMakeLists.txt to process
#add_subdirectory(Logger)    # look in apps subdirectory for CMakeLists.txt to process