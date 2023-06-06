cmake_minimum_required(VERSION 3.10)


if(${DEBUG_BUILD})
    message("[+]DEBUG BUILD!")
    set(CMAKE_BUILD_TYPE Debug)
    add_compile_definitions(DEBUG_BUILD)
else()
    message("[+]RELEASE BUILD!")
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_CXX_FLAGS_RELEASE "-o3 -DNDEBUG")
endif()

add_compile_definitions(QUAINT_PLATFORM_${BUILD_PLATFORM})

# Module Name
project(Bolt VERSION 1.0)

set(BUILD_CORE_LIB ON)

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE BOLT_SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)

if(${BUILD_BOLT_LIB})
    message("[+]Building Bolt Library")
    add_library(Bolt ${BOLT_SOURCES})

else()
    message("[+]Building Bolt Executable")
    
    add_executable(Bolt ${BOLT_SOURCES})
    
    include("${CMAKE_CURRENT_LIST_DIR}/../Core/CMakeLists.txt")
    message(... ${BOLT_SOURCES})
    
    # Module Name
    project(Bolt VERSION 1.0)

endif()

# Make headers available to other libraries
include_directories(${WORKSPACE_ROOT}/External/Libs/Vulkan-1.2.176.1/Include)
target_include_directories(Bolt PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)

add_dependencies(Bolt Core)
target_link_libraries(Bolt ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Core.lib)

if ("${BUILD_PLATFORM}" STREQUAL "WIN32")
    message("[+]Linked 32-bit Windows vulkan Library")
    target_link_libraries(Bolt ${WORKSPACE_ROOT}/External/Libs/Vulkan-1.2.176.1/Lib32/vulkan-1.lib)
elseif("${BUILD_PLATFORM}" STREQUAL "WIN64")
    message("[+]Linked 64-bit Windows vulkan Library")
    target_link_libraries(Bolt ${WORKSPACE_ROOT}/External/Libs/Vulkan-1.2.176.1/Lib/vulkan-1.lib)
else()
    message("[-]Vulkan Library could not be linked")
endif()
#message("[+]Registered headers under:" ${CMAKE_CURRENT_LIST_DIR}/Include)
#set(${PROJECT_NAME}_HEADER_PATH ${CMAKE_CURRENT_LIST_DIR}/Include)

#TODO: Register sub modules here
#add_subdirectory(Renderer)   # look in hello subdirectory for CMakeLists.txt to process
#add_subdirectory(Logger)    # look in apps subdirectory for CMakeLists.txt to process