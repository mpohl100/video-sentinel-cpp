cmake_minimum_required(VERSION 3.0.0)
project(evol VERSION 0.1.0)

find_program(CONAN conan)
if (NOT CONAN)
    message(FATAL_ERROR "Needs conan package manager: please visit https://conan.io and install")
endif ()

#include(${CMAKE_SOURCE_DIR}/scripts/conan2.cmake)

#conan_check(VERSION 1.14.4 REQUIRED)
#conan_cmake_settings(settings)
#conan_cmake_autodetect(settings)
#conan_add_remote(NAME conancenter INDEX 1
#        URL https://center.conan.io
#        VERIFY_SSL True)
#conan_add_remote(NAME bincrafters INDEX 2
#        URL https://api.bintray.com/conan/bincrafters/public-conan
#        VERIFY_SSL True)
#conan_add_remote(NAME mpusz INDEX 3
#        URL https://api.bintray.com/conan/mpusz/conan-mpusz
#        VERIFY_SSL True)
#conan_cmake_run(CONANFILE conanfile.txt
#        BASIC_SETUP CMAKE_TARGETS
#        BUILD missing)

if( NOT CMAKE_BUILD_TYPE )
     set( CMAKE_BUILD_TYPE Release )
endif()

set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set( CMAKE_CXX_FLAGS_RELEASE "-O3")
set( CMAKE_CXX_FLAGS_DEBUG "-g") 

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif ()
else ()
    set (CMAKE_CXX_STANDARD 20)
endif ()

enable_testing()

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)