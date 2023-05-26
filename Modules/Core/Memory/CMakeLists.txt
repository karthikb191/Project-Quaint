cmake_minimum_required(VERSION 3.10)

# Module Name
#project(QuaintMemory VERSION 1.0)

# Compile features

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(NOT ${BUILD_CORE_LIB})
    #Module Name
    project(QuaintMemory VERSION 1.0)
    message("[+]Project Defined: " ${PROJECT_NAME})
endif()

if(NOT ${BUILD_MEMORY_LIB})
    message("[+]Building Quaint-Memory Executable")
    add_executable(${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})
elseif(NOT ${BUILD_CORE_LIB})
    message("[+]Building \"QuaintMemory\" Libray")
    list(FILTER SOURCES EXCLUDE REGEX ".*/Source/Main\\.cpp$")
    add_library(${PROJECT_NAME} STATIC)
else()
    list(FILTER SOURCES EXCLUDE REGEX ".*/Source/Main\\.cpp$")
    message("[+]Adding Quaint-Memory to core Library")
endif()

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

if(NOT ${BUILD_CORE_LIB})
    if(BUILD_LOGGER_LIB)
        target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintLogger.lib)
    endif()
    if(BUILD_TYPES_LIB)
        target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintTypes.lib)
    endif()
    if(BUILD_IPC_LIB)
        target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintIPC.lib)
    endif()
endif()