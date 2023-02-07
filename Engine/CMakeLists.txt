cmake_minimum_required(VERSION 3.10)

if(${DEBUG_BUILD} EQUAL 1)
     message("[+]Debug Build")
     set(CMAKE_CXX_FLAGS_RELEASE "-O0 -g")
else()
     message("[+]Non-Debug Build")
endif()

#Project variables to be passed on to child CMakeLists.txt
set(BUILD_BOLT_LIB ON)
set(BUILD_CORE_LIB ON)


#Include External Modules(CMakeLists.txt) here
include("${CMAKE_CURRENT_LIST_DIR}/../Modules/Bolt/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/../Modules/Core/CMakeLists.txt")


#Include External plugins here



# Module Name. The root project must be set after all the other CMakeLists to give it a proper .sln name
project(Quaint)

#Source files
#TODO: Generate CMakelists through an external script once build times start to become unbearable
file(GLOB_RECURSE SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp"
)
add_executable(Quaint ${SOURCES})



#Setup Links to other modules and plugins here
target_link_libraries(Quaint PUBLIC Bolt)
target_link_libraries(Quaint PUBLIC Core)


# Make headers available to other libraries
target_include_directories(Quaint PUBLIC 
                        ${CMAKE_CURRENT_LIST_DIR}/Include
                        )

#TODO: Register sub modules here
#add_subdirectory(Renderer)   # look in hello subdirectory for CMakeLists.txt to process
#add_subdirectory(Logger)    # look in apps subdirectory for CMakeLists.txt to process