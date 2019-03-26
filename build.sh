#!/bin/bash
set -ue
cd "$(dirname "$0")"

BUILD_TYPE=Release # or Debug
if [[ $(uname -s) == CYGWIN* ]] || [[ $(uname -s) == MINGW* ]]; then
    WINDOWS=true
fi

if [ ! -z ${WINDOWS+x} ]; then
	mkdir VSProject || true
	cd VSProject
	cmake -G "Visual Studio 15 2017" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -A x64 -Thost=x64 -DLLVM_SRC_DIR=$(realpath ../llvm) -DLLVM_DIR=$(realpath ../llvm-build/lib/cmake/llvm) -DZLIB_INCLUDE_DIR=$(realpath ../zlib) -DZLIB_LIBRARY=$(realpath ../zlib-build/$BUILD_TYPE/zlibstatic.lib) ..
	cmake --build . --target clang-sfi --config $BUILD_TYPE
else
	rm -Rf build
	mkdir build
	cd build
	cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DLLVM_SRC_DIR=$(realpath ../llvm) -DLLVM_DIR=$(realpath ../llvm-build/lib/cmake/llvm) ..
	if [ -x "$(command -v nproc)" ]; then
		make -j$(nproc)
	else
		make -j
	fi
fi
