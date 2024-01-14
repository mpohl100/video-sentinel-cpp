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

conan profile detect --force

conan install .. --build=missing --output-folder=conan_output -pr="$(pwd)/../conan_profiles/gcc11_cpp20.prof" --update -c tools.system.package_manager:mode=install -c tools.system.package_manager:sudo=True 

cmake .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="$(pwd)/conan_output/build/Release/generators/conan_toolchain.cmake" -DCMAKE_POLICY_DEFAULT_CMP0091=NEW -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(pwd)/conan_output/build/Release/generators"

make
