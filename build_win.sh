#!/bin/bash
set -ue

TOP_LEVEL_DIRECTORY=$(dirname "$(realpath "$0")")
BUILD_TYPE=Release # or Debug

cd "$(dirname "$0")"

#rm -Rf VSProject
mkdir VSProject || true
cd VSProject
cmake -G "Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -A x64 -Thost=x64 -DLLVM_SRC_DIR=$(realpath ../llvm) -DLLVM_DIR=$(realpath ../llvm-build/lib/cmake/llvm) -DZLIB_INCLUDE_DIR=$(realpath ../zlib) -DZLIB_LIBRARY=$(realpath ../zlib-build/$BUILD_TYPE/zlibstatic.lib) ..
cmake --build . --target clang-sfi --config $BUILD_TYPE

