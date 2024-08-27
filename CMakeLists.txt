cmake_minimum_required(VERSION 3.0.0)
project(evol VERSION 0.1.0)

find_program(CONAN conan)
if (NOT CONAN)
    message(FATAL_ERROR "Needs conan package manager: please visit https://conan.io and install")
endif ()

if( NOT CMAKE_BUILD_TYPE )
     set( CMAKE_BUILD_TYPE Release )
endif()

set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set( CMAKE_CXX_FLAGS_RELEASE "-O3")
set( CMAKE_CXX_FLAGS_DEBUG "-O3")
set( CMAKE_CXX_FLAGS_DEBUG "-g") 

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
endif ()
else ()
    set (CMAKE_CXX_STANDARD 20)
endif ()

if(ENABLE_COVERAGE)
  message("coverage enabled")
  # set compiler flags
  set(CMAKE_CXX_FLAGS "-O0 -coverage")

  # find required tools
  find_program(LCOV lcov REQUIRED)
  find_program(GENHTML genhtml REQUIRED)

  # add coverage target
  add_custom_target(coverage
    # gather data
    COMMAND ${LCOV} --directory . --capture --output-file coverage.info
    # generate report
    COMMAND ${GENHTML} --demangle-cpp -o coverage coverage.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(ENABLE_CLANG_TIDY)
    message("clang_tidy enabled")
    set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")
    # Enable the generation of compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    add_custom_target(run_clang_tidy
        COMMAND ${CMAKE_COMMAND} -E echo "Running clang-tidy for all libraries and targets..."
        COMMAND find ${CMAKE_SOURCE_DIR} -name "*.cpp" | xargs clang-tidy -p ${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running clang-tidy checks for all libraries and executables"
    )
endif()

enable_testing()

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)