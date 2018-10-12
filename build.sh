#!/bin/bash
set -ue
cd "$(dirname "$0")"

rm -Rf build
mkdir build
cd build
cmake -G "Unix Makefiles" -DLLVM_SRC_DIR=$(realpath ../llvm) -DLLVM_DIR=$(realpath ../llvm-build/lib/cmake/llvm) ..
if [ -x "$(command -v nproc)" ]; then
	make -j$(nproc)
else
	make -j
fi

