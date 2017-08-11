
cmake_minimum_required(VERSION 2.8)
project(DocBaker C)

set(PACKAGE         "docbaker")

set(MAJOR_VERSION   0)
set(MINOR_VERSION   1)
set(PATCH_VERSION   0)
set(VERSION         ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION})


include(GNUInstallDirs OPTIONAL)
include(FindGettext OPTIONAL)


set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo MinSizeRel)
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE $ENV{CMAKE_BUILD_TYPE})

    if("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(CMAKE_BUILD_TYPE "Release")
    endif()
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()

add_subdirectory(src)
