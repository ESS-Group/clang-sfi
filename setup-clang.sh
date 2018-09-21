#!/bin/bash
TOP_LEVEL_DIRECTORY=$(dirname "$(realpath "$0")")
BUILD_TYPE=Release # or Debug

cd "$TOP_LEVEL_DIRECTORY"
echo "Switched into $(pwd)"
echo "Checking out clang into llvm/tools"
cd llvm/tools
git clone https://llvm.org/git/clang.git
(cd clang && git reset --hard 0513b409d5e34b2d2a28ae21b6d620cc52de0e57)

cd "$TOP_LEVEL_DIRECTORY"
echo "Switched into $(pwd)"
mkdir llvm-build
cd llvm-build
echo "Created llvm-build"
cmake -G Ninja ../llvm -DCMAKE_BUILD_TYPE=$BUILD_TYPE
echo "Preparing Ninja files"

COMPILE="n"
read -p "Should I start the compile? [$COMPILE]: " userInput
if [[ -n "$userInput" ]]
then
    COMPILE=$userInput
fi
if [[ $COMPILE =~ ^(yes|y|Y) ]]; then
    echo "Start compile"
    ninja
fi

