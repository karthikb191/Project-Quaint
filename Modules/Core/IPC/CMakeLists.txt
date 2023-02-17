cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(NOT ${BUILD_CORE_LIB})
    #Module Name
    project(QuaintIPC VERSION 1.0)
    message("[+]Project Defined: " ${PROJECT_NAME})
endif()

if(NOT ${BUILD_IPC_LIB})
    message("[+]Building Quaint-IPC Executable")
    add_executable(${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})
elseif(NOT ${BUILD_CORE_LIB})
    message("[+]Building \"Quaint-IPC\" Libray")
    list(FILTER SOURCES EXCLUDE REGEX ".*Source/Main\\.cpp$")
    add_library(${PROJECT_NAME} STATIC)
else()
    list(FILTER SOURCES EXCLUDE REGEX ".*Source/Main\\.cpp$")
    message("[+]Adding Quaint-IPC to core Library")
endif()

target_sources(${PROJECT_NAME} PUBLIC ${SOURCES})

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

if(NOT ${BUILD_CORE_LIB})
    if(${BUILD_TYPES_LIB})
        target_link_libraries(QuaintIPC PUBLIC QuaintTypes)
    endif()
    if(${BUILD_LOGGER_LIB})
        target_link_libraries(QuaintIPC PUBLIC QuaintLogger)
    endif()
    if(${BUILD_MEMORY_LIB})
        target_link_libraries(QuaintIPC PUBLIC QuaintMemory)
    endif()
endif()