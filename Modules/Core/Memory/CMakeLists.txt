cmake_minimum_required(VERSION 3.10)

# Module Name
project(QuaintMemory VERSION 1.0)

# Compile features

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(${BUILD_LIB})
    message("[+]Building Quaint-Memory Library")
    add_library(${PROJECT_NAME} ${SOURCES})
else()
    message("[+]Building Quaint-Memory Executable")
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

target_link_libraries(QuaintMemory PUBLIC QuaintTypes)