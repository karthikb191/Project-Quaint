cmake_minimum_required(VERSION 3.10)

# Module Name
project(QuaintTypes VERSION 1.0)

# Compile features

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

message("[+]Building Quaint-Types Library")
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)
