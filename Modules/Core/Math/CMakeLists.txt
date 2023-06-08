cmake_minimum_required(VERSION 3.10)

# Module Name
#project(QuaintMath VERSION 1.0)
set(LIB_TYPE "PUBLIC")

# Compile features

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(NOT ${BUILD_CORE_LIB})
    #Module Name
    project(QuaintMath VERSION 1.0)
    message("[+]Project Defined: " ${PROJECT_NAME})
endif()

if(NOT ${BUILD_MATH_LIB})
    message("[+]Building Quaint-Math Executable")
    add_executable(${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME})
elseif(NOT ${BUILD_CORE_LIB})
    message("[+]Building \"QuaintMath\" Libray")
    list(FILTER SOURCES EXCLUDE REGEX ".*/Source/Main\\.cpp$")
    
    if(SOURCES)
        add_library(${PROJECT_NAME} STATIC)
        set(LIB_TYPE "PUBLIC")
    else()
        set(LIB_TYPE "INTERFACE")
        add_library(${PROJECT_NAME} ${LIB_TYPE})
        message("QuaintMath is a header only library")
    endif()
else()
    list(FILTER SOURCES EXCLUDE REGEX ".*/Source/Main\\.cpp$")
    message("[+]Adding Quaint-Math to core Library")
endif()


target_sources(${PROJECT_NAME} ${LIB_TYPE} ${SOURCES})

# Make include directories available to use within this project
include_directories(${CMAKE_CURRENT_LIST_DIR}/Include)

# Make headers available to other libraries
target_include_directories(${PROJECT_NAME} ${LIB_TYPE} ${CMAKE_CURRENT_LIST_DIR}/Include)

#set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)
if(NOT ${BUILD_CORE_LIB})

    if(BUILD_LOGGER_LIB)
        add_dependencies(${PROJECT_NAME} QuaintLogger)
        message(${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintLogger.lib)
        target_link_libraries(${PROJECT_NAME} ${LIB_TYPE} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintLogger.lib)
    endif()
    if(BUILD_MEMORY_LIB)
        add_dependencies(${PROJECT_NAME} QuaintMemory)
        target_link_libraries(${PROJECT_NAME} ${LIB_TYPE} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintMemory.lib)
    endif()
    if(BUILD_TYPES_LIB)
        add_dependencies(${PROJECT_NAME} QuaintTypes)
        target_link_libraries(${PROJECT_NAME} ${LIB_TYPE} ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/QuaintTypes.lib)
    endif()
endif()