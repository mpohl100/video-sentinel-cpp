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

conan install .. --build="missing" --output-folder=conan_output -pr:a="$(pwd)/../conan_profiles/gcc11_cpp20_debug.prof" --update -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True 

conan build .. -b="missing" -pr:b="$(pwd)/../conan_profiles/gcc11_cpp20.prof"