#!/bin/bash

# Set default build directory
BUILD_DIR=$(pwd)/../build

# Check if a build directory is provided as a command line argument
if [ "$#" -gt 0 ]; then
    BUILD_DIR="$1"
fi

# Create the build directory if it doesn't exist
mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

rm -rf Debug
conan build .. --options clang_tidy=True -b="missing" -pr:b="$(pwd)/../conan_profiles/clang14_cpp20_debug.prof" -pr:h="$(pwd)/../conan_profiles/clang14_cpp20_debug.prof" -s build_type=Debug -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True 

cd Debug
make run_clang_tidy