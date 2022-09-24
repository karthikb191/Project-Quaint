cmake_minimum_required(VERSION 3.10)

# Module Name
project(Bolt VERSION 1.0)

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(${BUILD_BOLT_LIB})
    message("[+]Building Bolt Library")
    add_library(Bolt ${SOURCES})
else()
    message("[+]Building Bolt Executable")
    add_executable(Bolt ${SOURCES})
endif()

# Make headers available to other libraries
target_include_directories(Bolt PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

#TODO: Register sub modules here
#add_subdirectory(Renderer)   # look in hello subdirectory for CMakeLists.txt to process
#add_subdirectory(Logger)    # look in apps subdirectory for CMakeLists.txt to process