#!/bin/bash
set -ue

TOP_LEVEL_DIRECTORY=$(dirname "$(realpath "$0")")
BUILD_TYPE=Release # or Debug

cd "$TOP_LEVEL_DIRECTORY"
echo "Switched into $(pwd)"
echo "Checking out clang into llvm/tools"
cd llvm/tools
git clone https://llvm.org/git/clang.git || true
(cd clang && git reset --hard 0513b409d5e34b2d2a28ae21b6d620cc52de0e57)


cd "$TOP_LEVEL_DIRECTORY"
echo "Checking out zlib"
git clone https://github.com/madler/zlib.git || true
(cd zlib && git reset --hard cacf7f1d4e3d44d871b605da3b647f07d718623f)


cd "$TOP_LEVEL_DIRECTORY"
echo "Switched into $(pwd)"
mkdir zlib-build || true
cd zlib-build
cmake -G "Visual Studio 15 2017" -A x64 -Thost=x64 -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../zlib
echo "Building zlib"
cmake --build . --target ALL_BUILD --config $BUILD_TYPE
cd ..


mkdir -p llvm-build || true
cd llvm-build
echo "Created llvm-build"
#-DZLIB_SRC_DIR=$(realpath ../zlib) -DZLIB_DIR=$(realpath ../zlib-build/$BUILD_TYPE)
cmake -G "Visual Studio 15 2017" -A x64 -Thost=x64 -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DZLIB_INCLUDE_DIR=$(realpath ../zlib) -DZLIB_LIBRARY=$(realpath ../zlib-build/$BUILD_TYPE/zlibstatic.lib) ../llvm
cmake --build . --target ALL_BUILD --config $BUILD_TYPE 
#echo "Preparing Ninja files"
#cmake --build .
cd ..




#COMPILE="n"
#read -p "Should I start the compile? [$COMPILE]: " userInput
#if [[ -n "$userInput" ]]
#then
#    COMPILE=$userInput
#fi
#if [[ $COMPILE =~ ^(yes|y|Y) ]]; then
#    echo "Start compile"
#    ninja
#fi

