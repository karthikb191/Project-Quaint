cmake_minimum_required(VERSION 3.10)

# Compile features

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(NOT ${BUILD_CORE_LIB})
    #Module Name
    project(QuaintLogger VERSION 1.0)
    message("[+]Project Defined: " ${PROJECT_NAME})
endif()

if(NOT ${BUILD_LOGGER_LIB})
    message("[+]Building Quaint-Logger Executable " ${BUILD_LOGGER_LIB})
    add_executable(${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})
elseif(NOT ${BUILD_CORE_LIB})
    message("[+]Building \"QuaintLogger\" Libray")
    list(FILTER SOURCES EXCLUDE REGEX ".*Source/Main\\.cpp$")
    add_library(${PROJECT_NAME} STATIC)
else()
    list(FILTER SOURCES EXCLUDE REGEX ".*Source/Main\\.cpp$")
    message("[+]Adding Quaint Logger to core Library")
endif()

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

#TODO: Register sub modules here
#add_subdirectory(Renderer)   # look in hello subdirectory for CMakeLists.txt to process
#add_subdirectory(Logger)    # look in apps subdirectory for CMakeLists.txt to process